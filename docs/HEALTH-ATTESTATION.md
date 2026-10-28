# Deployment health attestation (v1)

A porta /v1/internal/deployments/:id/health exige **dois fatores de identidade de serviço**:

1. Bearer AGENT_SHARED_TOKEN (minimo 32 caracteres) no canal privado.
2. Assinatura Ed25519 válida gerada **exclusivamente** pela sonda confiável do
   ambiente sobre os campos e deploymentId de destino. A chave de verificação
   HEALTH_PROOF_PUBLIC_KEY_SPKI_B64 é DER SPKI codificada em base64; nunca expor
   a chave privada ou configurar um cliente/browser como sonda.

Payload exato em bytes UTF-8 (line feed, sem LF no final):
```text
pw-gameplay-health-v1
<deployment-uuid>
<checkedAt-ISO8601-with-offset>
<daemonsReady:true|false>
<gsReady:true|false>
<loginAndEnterWorldPassed:true|false>
<evidenceUrl-https>
```

Requisição JSON: checkedAt, daemonsReady, gsReady, loginAndEnterWorldPassed,
evidenceUrl, signature (base64 Ed25519). Id e hora fazem parte do payload
assinado: idade máxima 20s, rejeita replay tardio e cross-deployment.
O backend registra audit event e marca READY apenas quando os três critérios
forem verdadeiros. Expiração da observação do deployment continua 30s.

**A sonda real AINDA PRECISA SER IMPLEMENTADA:** a implementação da API verifica
atestação criptográfica, mas não prova que o emissor executou um login GNET;
o dono da chave privada é o único autorizado a atestar a prova. O CI pode usar
uma chave efêmera de teste para validar este contrato com fixture sintética.
Em produção, evitar usar a mesma chave para vários ambientes. Rotacionar e
restringir acesso da chave à sonda com attestation e rede privadas.
