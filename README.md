# Proyecto 1 - Chat por Sockets en C

##  Introducción

Este proyecto consiste en la implementación de un sistema de chat cliente-servidor utilizando sockets en lenguaje C, cumpliendo con los requisitos del curso de Telemática. El objetivo principal es permitir la comunicación en tiempo real entre múltiples usuarios a través de una red, aplicando conceptos fundamentales de programación en red, concurrencia con hilos (pthread) y el protocolo TCP/IP.

---

## Desarrollo

###  Estructura del Proyecto

El proyecto está compuesto por tres archivos principales:

- server.c: Código fuente del servidor. 
- client.c: Código fuente del cliente. 
- Makefile: Facilita la compilación de ambos componentes mediante los comandos make, make server, make client y make clean.

###  Flujo de conexión

1. El cliente se conecta al servidor usando TCP y envía su nombre de usuario.
2. El servidor guarda esta conexión, asigna un hilo para atender al cliente, y transmite su entrada al resto de los usuarios.
3. Los mensajes se muestran en tiempo real a todos los clientes conectados.
4. El servidor mantiene un archivo de log (chat_server.log) con registros de conexiones, mensajes y desconexiones.

###  Concurrencia

Se utiliza la biblioteca pthread para manejar múltiples clientes simultáneamente. Cada cliente es gestionado en un hilo independiente, lo que permite una comunicación fluida y sin bloqueos.

###  Entorno de despliegue

- El servidor se ejecuta en una instancia EC2 de AWS con Ubuntu 22.04.
- El cliente se prueba desde una máquina virtual con VirtualBox corriendo Linux.
- La conexión entre cliente y servidor se realiza a través de la IP pública de la instancia EC2.

---

##  Aspectos logrados

-  Comunicación en tiempo real entre múltiples clientes.
-  Manejo de concurrencia con pthread.
-  Registro de eventos en un archivo de logs.
-  Cliente funcional con validación de nombre de usuario.
-  Cierre limpio de sesión con comando exit.
-  Despliegue exitoso del servidor en la nube (AWS EC2).
-  Cliente conectado desde múltiples terminales VirtualBox.

---

##  Aspectos no logrados

-  Reconexión automática de clientes.
-  No hay base de datos donde guardar los chats.

---

##  Conclusiones

Este proyecto permitió comprender a fondo cómo funciona la comunicación cliente-servidor mediante sockets en C. Además, se aplicaron buenas prácticas en el manejo de múltiples conexiones, uso de hilos, y despliegue real en un entorno de nube.

---

## Referencias

- Curso ST0255 - Telemática
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)
- Documentación oficial de POSIX Sockets
- Manuales man de Linux (man socket, man bind, man pthread_create, etc.)