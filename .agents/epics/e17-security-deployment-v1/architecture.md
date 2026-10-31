# Arquitetura — Segurança, infraestrutura, CI/CD e entrega

Estado: planejado. O desenho específico abaixo substitui interfaces genéricas
sem alterar o estado das workorders. Cenário: candidato sintético validado sobe
para R2, ganha Preview, passa smoke e só então pode ser promovido por humano.

App Worker detém bootstrap/pin, Static Assets detém shell; R2/CDN detém bytes
imutáveis. Publisher é o único escritor da distribuição. CI de PR não recebe
secrets. Realm autoritativo continua sob contratos e10/e11, sem backend inventado.

## Contratos

- [ReleaseManifest/assinatura/promoção/rollback](contracts/release-manifest.md).
- [Build, source portada e pipeline GitHub](build-pipeline.md).
- [Cloudflare, config, HTTP/CDN, ambientes e CLI](cloudflare.md).
- [Conteúdo e15](../e15-assets-streaming-perf-v1/contracts/content-manifest.md).

Não há transação distribuída entre Worker/R2/realm. Publicar bytes antes do pin
não remove sessões antigas; elas conservam sua release. Rollback do Worker não
desfaz dados ou schema do servidor. GC remoto fica desativado até provar leases
e retenção de versões. Negativos de hash, stale SHA, secrets e mistura de
ambientes fazem parte do aceite de [CF-01–24](delivery-tasks.md).

[Visão integrada](../../../docs/CLOUDFLARE-DELIVERY.md).
