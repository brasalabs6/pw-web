use axum::{extract::{ws::{Message, WebSocket, WebSocketUpgrade}, State},http::StatusCode,response::IntoResponse,routing::get,Json,Router};
use futures_util::{SinkExt, StreamExt};
use serde::{Deserialize,Serialize};
use std::{net::SocketAddr,sync::Arc,time::Duration};
use tokio::{io::{AsyncReadExt,AsyncWriteExt},net::{TcpListener,TcpStream},time::timeout};

#[derive(Clone)]
struct Config {
    control_api: String,
    control_token: String,
    deployment_id: String,
    game_upstream: String,
    http: reqwest::Client,
}
#[derive(Deserialize)]
struct FirstMessage{ticket:String}
#[derive(Serialize)]
struct TicketConsume<'a>{ticket:&'a str,#[serde(rename="deploymentId")]deployment_id:&'a str}
#[derive(Deserialize)]
struct TicketResult{ok:bool}

async fn ready(State(cfg):State<Arc<Config>>)->impl IntoResponse{
    match timeout(Duration::from_millis(600),TcpStream::connect(&cfg.game_upstream)).await {
        Ok(Ok(_)) => (StatusCode::OK,Json(serde_json::json!({"transport":"reachable","gameplayReady":false}))),
        _ => (StatusCode::SERVICE_UNAVAILABLE,Json(serde_json::json!({"transport":"unreachable","gameplayReady":false})))
    }
}
async fn health()->impl IntoResponse{Json(serde_json::json!({"service":"pw-game-gateway","status":"alive"}))}
async fn upgrade(ws:WebSocketUpgrade,State(cfg):State<Arc<Config>>)->impl IntoResponse{
    ws.max_message_size(1024*1024).max_frame_size(1024*1024)
      .on_upgrade(move|socket|bridge(socket,cfg))
}
async fn bridge(mut ws:WebSocket,cfg:Arc<Config>){
    // Browser sends the opaque, single-use ticket as its first WebSocket message.
    // No query parameter tokens; no arbitrary client-supplied TCP upstream addresses.
    let first=match timeout(Duration::from_secs(8),ws.recv()).await {
        Ok(Some(Ok(Message::Text(value))))=>value,
        _=>{let _=ws.close().await;return}
    };
    let ticket=match serde_json::from_str::<FirstMessage>(&first) {
        Ok(x) if x.ticket.len()>=32 && x.ticket.len()<=256 =>x.ticket,
        _=>{let _=ws.close().await;return}
    };
    let endpoint=format!("{}/v1/internal/tickets/consume",cfg.control_api.trim_end_matches('/'));
    let response=cfg.http.post(endpoint).bearer_auth(&cfg.control_token)
      .json(&TicketConsume{ticket:&ticket,deployment_id:&cfg.deployment_id})
      .send().await;
    let authorized=match response{
        Ok(r) if r.status().is_success()=>r.json::<TicketResult>().await.map(|v|v.ok).unwrap_or(false),
        _=>false
    };
    if !authorized{let _=ws.close().await;return}
    let tcp=match timeout(Duration::from_secs(3),TcpStream::connect(&cfg.game_upstream)).await{
        Ok(Ok(s))=>s,_=>{let _=ws.close().await;return}
    };
    tracing::info!("Accepted authenticated GNET transport session");
    let (mut tcp_rx,mut tcp_tx)=tcp.into_split();
    let (mut ws_tx,mut ws_rx)=ws.split();
    let to_game=async{
        while let Some(message)=ws_rx.next().await{
            match message {
                Ok(Message::Binary(bytes))=>{
                    if tcp_tx.write_all(&bytes).await.is_err(){break}
                }
                Ok(Message::Close(_))|Err(_)=>break,
                Ok(Message::Text(_))=>break, // after auth only GNET bytes are permitted
                _=>{}
            }
        }
        let _=tcp_tx.shutdown().await;
    };
    let to_client=async{
        let mut buf=[0u8;32*1024];
        loop{
            let n=match tcp_rx.read(&mut buf).await{Ok(0)|Err(_)=>break,Ok(n)=>n};
            if ws_tx.send(Message::Binary(buf[..n].to_vec())).await.is_err(){break}
        }
        let _=ws_tx.send(Message::Close(None)).await;
    };
    tokio::select!{_ = to_game=>{}, _ = to_client=>{}}
}
#[tokio::main]
async fn main()->Result<(),Box<dyn std::error::Error>>{
    tracing_subscriber::fmt().with_env_filter("info").init();
    let token=std::env::var("GATEWAY_CONTROL_TOKEN").expect("GATEWAY_CONTROL_TOKEN required");
    let deployment_id=std::env::var("GATEWAY_DEPLOYMENT_ID").expect("GATEWAY_DEPLOYMENT_ID required");
    if token.len()<32{panic!("Gateway token must have at least 32 characters")}
    if !deployment_id.chars().all(|c|c.is_ascii_hexdigit()||c=='-'){panic!("Invalid deployment id")}
    let config=Arc::new(Config{
        control_api:std::env::var("CONTROL_API_URL").unwrap_or("http://127.0.0.1:8787".into()),
        control_token:token,
        deployment_id,
        game_upstream:std::env::var("GNET_UPSTREAM").unwrap_or("127.0.0.1:29000".into()),
        http:reqwest::Client::builder().timeout(Duration::from_secs(5)).build()?
    });
    let addr:SocketAddr=std::env::var("GATEWAY_LISTEN")
        .unwrap_or("127.0.0.1:9001".into()).parse()?;
    let app=Router::new().route("/healthz",get(health)).route("/readyz",get(ready))
        .route("/connect",get(upgrade)).with_state(config);
    tracing::info!("Game gateway on {}",addr);
    let listener=TcpListener::bind(addr).await?;
    axum::serve(listener,app).await?;
    Ok(())
}
