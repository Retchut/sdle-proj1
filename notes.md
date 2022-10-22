Posts guardados em /storage/ClientID/topic/postID e /storage/server/topic/postID

SUBSCRIBE: \
  Mensagem: `SUB <clientID> <topic>` \
  resposta do servidor: `SUB <clientID> <topic>` | `RESUB <clientID> <topic>`\

UNSUBSCRIBE:\
  Mensagem: `UNSUB <clientID> <topic>` \
  resposta do servidor: `UNSUB <clientID> <topic>`\

GET:\
  Mensagem: `GET <clientID> <topic> <last post ID>` \
  resposta do servidor: `GET <postid> <conteúdo eventual>` | `error`\

PUT:\
  Mensagem: `PUT <clientID> <topic> <conteúdo eventual>` \
  resposta do servidor: `PUT <clientID> <topic> <conteúdo eventual>`\