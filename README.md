# TCP-DAYTIME
 Servidor concurrente y cliente de protocolo DAYTIME usando TCP y escrito en C. Proyecto desarrollado con fines de aprendizaje en Arquitectura de Redes y Servicios asignatura de Ingeniería de Software en la Universidad de Valladolid.


## Requisitos
- [Docker](https://www.docker.com/) 
## Desarrollo

Primero, configure el entorno:
```bash
git clone https://github.com/Sergio-MB/TCP-DAYTIME # Clonar repositorio
cd TCP-DAYTIME # Accedemos a dicho repositorio
docker run --name=ubuntu-daytime-tcp -it -v $(PWD):/home ubuntu

# Dentro del contenedor. Obtenga las herramientas del compilador de C
apt-get update
apt-get install build-essential
```

Para las próximas veces, teniendo el contenedor ya en mi máquina, ¿cómo implementar el entorno local?
```bash
docker start ubuntu-daytime-tcp
docker exec -it ubuntu-daytime-tcp bash
```

Compilar cliente y servidor:
```bash
# Dentro del contenedor
cd /home
gcc -Wall -o client.out daytime-tcp-client-Munumer-Blazquez.c
gcc -Wall -o server.out daytime-tcp-server-Munumer-Blazquez.c
```

Ejecutar server:
```bash
# Dentro del contenedor
./server.out [port-number] # port-number is optional
```

Para ejecutar el cliente necesitamos otra terminal:
```bash
docker exec -it ubuntu-daytime-tcp bash

# Dentro del contenedor
cd /home
./client.out [server.IP.address] [port-number] # server.IP.address obligatorio, port-number opcional
```
