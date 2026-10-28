use axum::{extract::{Path,State},http::{HeaderMap,StatusCode},routing::{get,post},Json,Router};
use serde::{Deserialize,Serialize};
use std::{collections::BTreeSet,net::SocketAddr,sync::Arc,time::Duration};
use subtle::ConstantTimeEq;
use tokio::{net::TcpListener,process::Command,time::timeout};

#[derive(Clone)]
struct Config{token:String,units:BTreeSet<String>,systemctl:String}
#[derive(Serialize)]
struct ServiceStatus{name:String,systemd_status:String}
#[derive(Deserialize)]
struct ActionRequest{reason:String}
fn valid_unit(unit:&str)->bool{
    unit.starts_with("pw-") && unit.ends_with(".service")
      && unit.chars().all(|c|c.is_ascii_alphanumeric() || ".-_".contains(c))
}
fn authorized(headers:&HeaderMap,cfg:&Config)->bool{
    let actual=headers.get("authorization").and_then(|v|v.to_str().ok()).unwrap_or("");
    let expected=format!("Bearer {}",cfg.token);
    actual.as_bytes().ct_eq(expected.as_bytes()).unwrap_u8()==1
}
async fn systemctl(cfg:&Config,args:&[&str])->Result<String,StatusCode>{
    let output=timeout(Duration::from_secs(6),Command::new(&cfg.systemctl)
      .args(args).kill_on_drop(true).output()).await
      .map_err(|_|StatusCode::GATEWAY_TIMEOUT)?
      .map_err(|_|StatusCode::BAD_GATEWAY)?;
    if !output.status.success(){return Err(StatusCode::BAD_GATEWAY)}
    Ok(String::from_utf8_lossy(&output.stdout).trim().to_string())
}
async fn health()->Json<serde_json::Value>{
    Json(serde_json::json!({"service":"pw-realm-agent","status":"alive","gameplayReady":false}))
}
async fn services(State(cfg):State<Arc<Config>>,headers:HeaderMap)
 ->Result<Json<Vec<ServiceStatus>>,StatusCode>{
    if !authorized(&headers,&cfg){return Err(StatusCode::UNAUTHORIZED)}
    let mut list=Vec::new();
    for name in &cfg.units{
        let status=systemctl(&cfg,&["is-active",name]).await.unwrap_or("unknown".into());
        list.push(ServiceStatus{name:name.clone(),systemd_status:status});
    }
    Ok(Json(list))
}
async fn action(State(cfg):State<Arc<Config>>,headers:HeaderMap,
    Path((name,action)):Path<(String,String)>,Json(req):Json<ActionRequest>)
 ->Result<Json<serde_json::Value>,StatusCode>{
    if !authorized(&headers,&cfg){return Err(StatusCode::UNAUTHORIZED)}
    if !cfg.units.contains(&name){return Err(StatusCode::NOT_FOUND)}
    if !["start","stop","restart"].contains(&action.as_str()){return Err(StatusCode::BAD_REQUEST)}
    if req.reason.trim().len()<10 || req.reason.len()>500{return Err(StatusCode::BAD_REQUEST)}
    tracing::warn!(unit=%name,operation=%action,reason=%req.reason,"Authorized realm service operation");
    systemctl(&cfg,&[&action,&name]).await?;
    Ok(Json(serde_json::json!({"accepted":true,"service":name,"action":action,
      "note":"systemd command exited; GNET/map readiness must be probed separately"})))
}
#[tokio::main]
async fn main()->Result<(),Box<dyn std::error::Error>>{
    tracing_subscriber::fmt().with_env_filter("info").init();
    let token=std::env::var("AGENT_SHARED_TOKEN").expect("AGENT_SHARED_TOKEN required");
    if token.len()<32{panic!("Agent token must have at least 32 characters")}
    let raw=std::env::var("AGENT_ALLOWED_UNITS").unwrap_or_default();
    let units:BTreeSet<String>=raw.split(',').filter(|s|!s.trim().is_empty())
      .map(|s|s.trim().to_string()).collect();
    if units.is_empty(){tracing::warn!("No systemd units enabled, agent is read-only");}
    if units.iter().any(|s|!valid_unit(s)){
       panic!("Only explicit pw-*.service unit names are allowed")
    }
    let cfg=Arc::new(Config{token,units,
      systemctl:std::env::var("SYSTEMCTL_PATH").unwrap_or("/usr/bin/systemctl".into())});
    let addr:SocketAddr=std::env::var("AGENT_LISTEN").unwrap_or("127.0.0.1:9010".into()).parse()?;
    if !addr.ip().is_loopback(){panic!("Agent must bind to loopback; use secure tunnel/private network")}
    let app=Router::new().route("/healthz",get(health)).route("/v1/services",get(services))
      .route("/v1/services/:name/:action",post(action)).with_state(cfg);
    let listener=TcpListener::bind(addr).await?;
    tracing::info!("Realm agent on {}",addr);
    axum::serve(listener,app).await?;
    Ok(())
}

#[cfg(test)]
mod tests {
  use super::*;
  fn config()->Config{Config{token:"a".repeat(40),units:BTreeSet::new(),systemctl:"/usr/bin/systemctl".into()}}
  #[test]
  fn rejects_non_allowlisted_unit_names(){
    for name in ["ssh.service","pw-../../etc/passwd.service","pw-map;rm.service","pw-map.service/../ssh","pw-map.service\n"] {
      assert!(!valid_unit(name),"unsafe systemd unit accepted: {:?}",name);
    }
    assert!(valid_unit("pw-gs01.service"));
    assert!(valid_unit("pw-glinkd.service"));
  }
  #[test]
  fn requires_exact_bearer_secret(){
    let c=config();
    let mut headers=HeaderMap::new();
    assert!(!authorized(&headers,&c));
    headers.insert("authorization",("Bearer ".to_owned()+&c.token).parse().unwrap());
    assert!(authorized(&headers,&c));
    headers.insert("authorization","Bearer wrong".parse().unwrap());
    assert!(!authorized(&headers,&c));
  }
}
