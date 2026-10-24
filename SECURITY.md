# Segurança

Relate vulnerabilidades de forma privada aos mantenedores por canal de segurança da organização. Evite issues públicas com exploits, tokens ou dados de jogadores. Nunca suba chaves privadas nem arquivos `.env`.

A segurança da fase inicial limita-se ao scaffold. Antes de liberar serviço multiplayer são obrigatórios: TLS/WSS; validação de origem e sessão; limites de tamanho/ritmo de pacotes; timeouts; autenticação server-side; segregação entre gateway e `pw-link`; tratamento de dados pessoais; auditoria de uploads e artefatos.

Não exponha barramento interno, PostgreSQL, Redis/Dragonfly ou endpoints administrativos à internet.
