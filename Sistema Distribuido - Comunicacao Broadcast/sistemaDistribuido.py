from pydoc import cli
from re import I
from asyncio.windows_events import NULL
import subprocess
import socket
import re
import sys
import os
import pickle
import threading
import queue
import datetime
import time

SERVER_PORT = 8888
PACKAGE_SIZE = 500
HEADER_SIZE = 8

clients = set()
pacotesRecebidos = queue.Queue()
listaConectados = [] # Lista de clientes conectados no momento
conexoesAtivas = [] # Lista temporária onde são armazenadas as conexões verificadas pelo teste


def server(server_host):
    threading.Thread(target=loopServidor, args=(server_host,)).start()
    threading.Thread(target=checarConexao, args=(server_host,)).start()


def loopServidor(server_host):
    print('* Função do servidor inicializada *')
    print('* Servidor funcionando no endereço: '+ str(server_host))
    s = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
    s.bind((server_host,SERVER_PORT))

    threading.Thread(target=ReceberDadosLoopServidor,args=(s, pacotesRecebidos, server_host)).start()

    while True:
        data,addr = pacotesRecebidos.get()
        print(f"* Servidor: {data}")
        if data == "remontar":
            print("* Refazendo conexões")
            copia = clients.copy()
            clients.clear()
            for c in copia:
                try:
                    print(f"* Fazendo a reconexão com: {c}")
                    s.sendto("remontar".encode(),c)
                    s.sendto(pickle.dumps(clients),addr)
                    time.sleep(1)
                except:
                    pass


        if data.endswith("conectado") and addr not in clients:
            print(f"* Servidor: {addr} sendo adicionado à lista de endereços conectados")
            clients.add(addr)
            s.sendto("clienteConectado".encode(),(server_host,SERVER_PORT+1))
            continue

        if data.endswith('/sair') and addr in clients:
            print(f"* Servidor: {addr} sendo removido da lista de endereços conectados")
            clients.remove(addr)
            continue

def getNghb(current_server_ip,current_member_ip, direction='left'):
    current_member_index = current_server_ip.index(current_member_ip) if current_member_ip in current_server_ip else -1
    if current_member_index != -1:
        if direction == 'left':
            if current_member_index + 1 == len(current_server_ip):
                return current_server_ip[0]
            else:
                return current_server_ip[current_member_index + 1]
        else:
            if current_member_index - 1 == 0:
                return current_server_ip[0]
            else:
                return current_server_ip[current_member_index - 1]
    else:
        return None

# Fica ouvindo as mensagens de novas conexões no sistema
def ReceberDadosLoopServidor(sock, pacotesRecebidos, server_host):
    while True:
        try:
            data,addr = sock.recvfrom(1024)
            data = data.decode()
            if data == "conectado":
                clients.add(addr)
                sock.sendto("clienteConectado".encode(),(server_host,SERVER_PORT+1))
            if data == "listaDeConectados":
                print(f"* Servidor: Mandando lista de endereços para o endereço {addr}")
                sock.sendto(pickle.dumps(clients),addr)
            else:
                pacotesRecebidos.put((data,addr))
        except:
            pass

# Checa se os nós no sistema estão respondendo. Caso não responda, será enviada uma mensagem aos nós que sobraram para reconstruir a árvore
def checarConexao(server_host):
    socketConexoesAtivas = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
    socketConexoesAtivas.bind((server_host,SERVER_PORT+1))

    threading.Thread(target=RecebeDadosConexoesAtivas,args=(socketConexoesAtivas,)).start()
    while True:
        time.sleep(5)
        try:
            conexoesAtivas.clear()
            for c in clients:
                print(f"* Enviando teste de conexão para o endereço {c}")
                socketConexoesAtivas.sendto("manterConexao".encode(),c)
        except:
            print("* Erro de verificação")
        time.sleep(5)

        if len(conexoesAtivas) != len(clients):
            socketConexoesAtivas.sendto("remontar".encode(),(server_host,SERVER_PORT))
            print("* Erro ao encontrar todas as conexões ativas no momento. Refazendo conexões...")

# Fica ouvindo as mensagens dos clientes
def RecebeDadosConexoesAtivas(socket):
    while True:
        try:
            data,addr = socket.recvfrom(1024)
            data = data.decode()
            if data == "manterConexao":
                conexoesAtivas.append(addr)
                print(f"* Recebeu conexão do endereço {addr}")
            elif data == "clienteConectado":
                conexoesAtivas.append(addr)

            elif data == "clienteDesconectado":
                conexoesAtivas.remove(addr)
        except:
            pass

def getAddrPing(conectados):
        addr_with_lowest_latency = 0
        lowest_latency = 0

        for addr in conectados:
            # ping
            response = str(subprocess.check_output(['ping', '-c', '1', addr]))
            latency = re.search(r'(\d+.\d+/){3}\d+.\d+', response).group(0)
            latency = latency.split('/')[0]

            if lowest_latency == NULL:
                lowest_latency = latency
            else:
                if latency < lowest_latency:
                    lowest_latency = latency
                    addr_with_lowest_latency = addr

        return addr_with_lowest_latency
   

def client(client_ip, client_port, server_ip):
    print('* Função do client inicializada *')
    PAYLOAD_SIZE = PACKAGE_SIZE - HEADER_SIZE
    sock = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
    HOST = client_ip
    PORT = int(client_port)
    
    sock.bind((HOST,PORT))

    server = (server_ip,SERVER_PORT)

    print(f"* Socket iniciado em: {sock.getsockname()}")

    name = input('* Nome de usuário: ')

    ## Pede ao servidor uma lista com todos os endereços que participam do sistema
    sock.sendto("listaDeConectados".encode(),server) 

    conectados, server_addr = sock.recvfrom(1024)
    conectados = pickle.loads(conectados)
    Connect(sock, conectados, server)

    threading.Thread(target=RecebeDados,args=(sock,server)).start()
    package_id = 1
    while True:
        data = input()
        # Envia a mensagem de desconectar para si mesmo para chamar a função de desconectar
        if data == '/sair': 
            sock.sendto("autoDesconectar".encode(),(HOST,PORT))
            break
        elif data == '':
            continue
        starttime = datetime.datetime.now()
        package_count = 0
        header = bytes('{:0>8}'.format(format(package_id, 'X')), 'utf-8') #Faz um header com caracteres com um identificador único para cada pacote, em hexadecimal
        package_id += 1
        data = ' Mensagem de '+ name +': ' + data
        ## Repassa a mensagem digitada para todos os endereços que estão na lista de conectados
        for c in listaConectados: 
            sock.sendto(data.encode(),c)
        sock.sendto(data.encode(),server)

    sock.sendto(data.encode(),server)
    time.sleep(5)
    sock.close()
    os._exit(1)

# Varre a lista de usuários conectados e procura o nó com a menor latência para estabelecer uma conexão com o mesmo.
# Se não existir nenhum nó, o servidor é informado e fica aguardando novas conexões
def Connect(sock: socket, conectados, server):

    # Nenhum usuario na lista de conectados
    if len(conectados) == 0: 
        sock.sendto("conectado".encode(),server)

    # Conecta com o nó de menor latência
    else: 
        menorLatencia = 100000
        addrMenorLatencia = ()
        for c in conectados: 
            starttime = time.time()
            sock.sendto("latencia".encode(),c)
            sock.recvfrom(1024)
            endtime = time.time()
            delta = starttime - endtime
            if delta < menorLatencia:
                menorLatencia = delta
                addrMenorLatencia = c
        sock.sendto("conectar".encode(),addrMenorLatencia) 
        print(f"* Conexão realizada com o endereço {addrMenorLatencia}")
        listaConectados.append(addrMenorLatencia)

        ## Avisa ao servidor que se conectou ao sistema
        sock.sendto("conectado".encode(),server) 


def RecebeDados(sock,server):
    while True:
        try:
            data,addr = sock.recvfrom(1024)
            dataDecoded = data.decode()

            if dataDecoded == "remontar":
                print("* Fazendo a reconexão com o servidor")
                listaConectados.clear()

                ## Pede ao servidor uma lista com todos os endereços que participam do sistema
                sock.sendto("listaDeConectados".encode(),server) 

                conectados, server_addr = sock.recvfrom(1024)
                conectados = pickle.loads(conectados)
                Connect(sock,conectados, server)

            elif dataDecoded == "manterConexao":
                print("* Recebendo verificação de conexão do servidor")
                sock.sendto("manterConexao".encode(),addr)

            elif dataDecoded == "conectar":
                print(f"* Conexão realizada com o endereço {addr}")
                listaConectados.append(addr)

            elif dataDecoded == "latencia":
                print(f"* Testando latência com o endereço {addr}")
                sock.sendto("latencia".encode(),addr)
            
            elif dataDecoded == "autoDesconectar":
                Disconnect(sock)

            elif dataDecoded == "desconectar":
                print(f"* A conexão com o endereço {addr} foi desfeita")
                ## Remove o endereço que enviou a mensagem da lista de conectados
                listaConectados.remove(addr) 
                data, addr = sock.recvfrom(1024)
                enderecosRestantes = pickle.loads(data)

                ## Loop que se conecta com os endereços que estavam conectados ao endereço que foi desligado
                for addr in enderecosRestantes: 
                    print(f"* Conexão realizada com o endereço {addr}")
                    sock.sendto("conectar".encode(),addr)
                    listaConectados.append(addr)
            
            elif dataDecoded == "desconectado":
                print(f"* A conexão com o endereço {addr} foi desfeita")
                listaConectados.remove(addr)

            # Repassa a mensagem recebida para a lista de conectados
            else:
                print(f" Mensagem recebida de {addr}: {dataDecoded}")
                for c in listaConectados:
                    if c!= addr:
                        sock.sendto(data,c)
        except:
            pass

# Desconecta do sistema mandando a lista de conectados para o primeiro endereço, fazendo ele se conectar com os demais
# E então os outros endereços removem o que está sendo removido do sistema de suas listas de conectados
def Disconnect(sock: socket):
    if len(listaConectados) == 0:
        return
    primeiroEndereco = listaConectados[0]
    listaConectados.remove(primeiroEndereco)
    sock.sendto("desconectar".encode(),primeiroEndereco) 

    ## Aqui é enviada a lista com os demais endereços conectados para o primeiro endereço
    sock.sendto(pickle.dumps(listaConectados),primeiroEndereco) 

    ## Aqui é enviada a mensagem para os demais endereços sinalizando uma saida do sistema
    for c in listaConectados:
        sock.sendto("desconectado".encode(),c) 


def main():
    
    argvlen = len(sys.argv)
    if (argvlen < 2) or (argvlen < 4 and sys.argv[1] == "client"):
        print('Formato inválido')
        print(f"Exemplo: python {sys.argv[0]}.py server ou client. Se for inserido server, insira apenas o ip do servidor. Caso for inserido o client, insira primeiro o ip do client, a porta e o ip do servidor\n")
        quit(1)

    # argumentos passados no terminal
    tipo = sys.argv[1]
    server_ip = sys.argv[2]
    client_ip = None
    if argvlen > 3:
        client_ip = sys.argv[2]
        client_port = sys.argv[3]
        server_ip = sys.argv[4]
    
    # execução do sistema
    if tipo == "server":
        server(server_ip)
    elif tipo == "client":
        client(client_ip, client_port, server_ip)
    else:
        print("Apenas client ou server.")
        quit(1)
        
if __name__ == "__main__":
    main()