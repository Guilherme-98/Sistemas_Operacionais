Alunos:
- Gabriel Angelo P Gasparini Sabaudo
- Guilherme Henrique Gonçalves Silva

Instalação do rpc:
apt-get install rpcbind

Biblioteca RPC independente de transporte
Este pacote contém uma porta da biblioteca RPC independente de transporte da Sun para Linux:
apt-get install -y libtirpc-dev

Para o funcionamento do programa, primeiro é necessário compilar o server e o client via makefile.

1. make server
2. make client

Depois é só executar o servidor e o client.

1. ./server
2. ./client (hostname), onde hostname pode ser um servidor como host ou o localhost

