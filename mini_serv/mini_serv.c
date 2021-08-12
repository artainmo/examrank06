#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>

typedef struct s_server
{
	int server_socket;
	struct sockaddr_in server_address;
	fd_set sockets;
	fd_set read_set;
	fd_set write_set;
	int max_fds;
} t_server;

typedef struct s_client
{
	int fd;
	int id;
	struct s_client *next;
} t_client;

void fatal(int id)
{
	write(2, "Fatal error\n", strlen("Fatal error\n"));
	//printf("%d\n", id);
	(void)id;
	exit(1);
}

int get_id(t_client *c, int fd)
{
	while (c != 0)
	{
		if (c->fd == fd)
			return (c->id);
		c = c->next;
	}
	write(2, "Error: get_id", strlen("Error: get_id"));
	exit(1);
}

void send_to_all_clients(t_server *s, t_client *c, char *text, int fd)
{
	while (c != 0)	
	{
		if (c->fd != fd && FD_ISSET(c->fd, &s->write_set))
		{
			if (send(c->fd, text, strlen(text), 0) == -1)
				fatal(8);
			//printf("To %d: %s", c->id, text);
		}
		c = c->next;
	}	
}

t_client *client_disconnection(t_server *s, t_client *c, int connected_socket)
{
	t_client *it;
	t_client *prior;
	char buffer[100];

	it = c;
	prior = 0;
	sprintf(buffer, "server: client %d just left\n", get_id(c, connected_socket));
	send_to_all_clients(s, c, buffer, connected_socket);
	while (it != 0)
	{
		if (it->fd == connected_socket)
		{
			if (prior == 0)
			{
				c = c->next;
				free(it);
				break ;
			}
			else
			{
				prior->next = prior->next->next;
				free(it);
				break ;
			}
		}
		prior = it;
		it = it->next;
	}
	FD_CLR(connected_socket, &s->sockets);
	close(connected_socket);
	return (c);
}

t_client *client_message(t_server *s, t_client *c, int connected_socket)
{
	char message[1000000];
	char line[1000000];
	char buffer[1000100];
	int len;
	int i;
	int l;

	i = 0;
	l = 0;
	if ((len = recv(connected_socket, message, sizeof(message) - 1, 0)) <= 0)
		c = client_disconnection(s, c, connected_socket);
	else
	{
		message[len] = 0;
		while (message[i])
		{
			line[l] = message[i];
			if (message[i] == '\n' || !message[i + 1])
			{
				line[l + 1] = 0;
				sprintf(buffer, "client %d: %s", get_id(c, connected_socket), line);
				send_to_all_clients(s, c, buffer, connected_socket);
				l = -1;
			}
			l++;
			i++;
		}
	}
	return (c);
}

t_client *client_connection(t_server *s, t_client *c)
{
	int new_connection;
	char buffer[100];
	t_client *it;
	socklen_t len;

	it = c;
	len = sizeof(s->server_address);
	if ((new_connection = accept(s->server_socket, (struct sockaddr *)&s->server_address, &len)) == -1)
		fatal(5);
	if (new_connection > s->max_fds)
		s->max_fds = new_connection;
	FD_SET(new_connection, &s->sockets);
	if (c == 0)
	{
		if ((c = malloc(sizeof(t_client *))) == 0)
			fatal(6);
		c->fd = new_connection;
		c->id = 0;
		c->next = 0;
	}
	else
	{
		while (it->next != 0)
			it = it->next;
		if ((it->next = malloc(sizeof(t_client *))) == 0)
			fatal(7);
		it->next->fd = new_connection;
		it->next->id = it->id + 1;
		it->next->next = 0;
	}
	sprintf(buffer, "server: client %d just arrived\n", get_id(c, new_connection));
	send_to_all_clients(s, c, buffer, new_connection);
	return (c);
}

void launch_server(t_server s)
{
	t_client *c;

	c = 0;
	s.max_fds = s.server_socket;
	FD_ZERO(&s.sockets);
	FD_SET(s.server_socket, &s.sockets);
	while (1)
	{
		s.read_set = s.write_set = s.sockets;
		if (select(s.max_fds + 1, &s.read_set, &s.write_set, 0, 0) == -1)
			fatal(4);
		if (FD_ISSET(s.server_socket, &s.read_set))
		{
			c = client_connection(&s, c);
			continue ;
		}
		for (int fd = 0; fd <= s.max_fds; fd++)
		{
			if (FD_ISSET(fd, &s.read_set))
				c = client_message(&s, c, fd);
		}
	}
}

void setup_server(t_server *s, int port)
{
	s->server_address.sin_family = AF_INET; 
	s->server_address.sin_addr.s_addr = htonl(2130706433); //127.0.0.1
	s->server_address.sin_port = htons(port); 
	if ((s->server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		fatal(1);  
	if (bind(s->server_socket, (const struct sockaddr *)&s->server_address, sizeof(s->server_address)) != 0)
		fatal(2);
	if (listen(s->server_socket, 0) != 0)
		fatal(3);
}

int main(int argc, char **argv)
{
	t_server s;

	if (argc == 1)
	{
		write(2, "Wrong number of arguments\n", strlen("Wrong number of arguments\n"));
		exit(1);
	}
	setup_server(&s, atoi(argv[1]));
	launch_server(s);
	return (0);
}
