#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <errno.h>

typedef struct s_server
{
	int server_socket;
	struct sockaddr_in server_address;
	fd_set sockets;
	fd_set read_set;
	fd_set write_set;
	int max_fds;
	int max_id;
} t_server;

typedef struct s_client
{
	int fd;
	int id;
	char *message_buffer;
	struct s_client *next;
} t_client;

void fatal(int id)
{
	write(2, "Fatal error\n", strlen("Fatal error\n"));
	//printf("%d : %s\n", id, strerror(errno));
	(void)id;
	exit(1);
}

char *str_join(char *buf, char *add)
{
    char    *newbuf;
    int     len;

    if (buf == 0)
        len = 0;
    else
        len = strlen(buf);
    newbuf = malloc(sizeof(*newbuf) * (len + strlen(add) + 1));
    if (newbuf == 0)
        return (0);
    newbuf[0] = 0;
    if (buf != 0)
        strcat(newbuf, buf);
    free(buf);
    strcat(newbuf, add);
    return (newbuf);
}

void send_to_all_clients(t_server *s, t_client *c, char *text, int fd)
{
	while (c != 0)	
	{
		if (c->fd != fd && FD_ISSET(c->fd, &s->write_set))
			send(c->fd, text, strlen(text), MSG_DONTWAIT); //MSG_DONTWAIT enables nonblocking operation
		c = c->next;
	}	
}

t_client *client_disconnection(t_server *s, t_client *start, t_client *connected_socket)
{
	t_client *it;
	t_client *prior;
	char buffer[100];

	it = start;
	prior = 0;
	sprintf(buffer, "server: client %d just left\n", connected_socket->id);
	send_to_all_clients(s, start, buffer, connected_socket->fd);
	while (it != 0)
	{
		if (it->fd == connected_socket->fd)
		{
			if (prior == 0)
				start = start->next;
			else
				prior->next = prior->next->next;
			break ;
		}
		prior = it;
		it = it->next;
	}
	FD_CLR(connected_socket->fd, &s->sockets);
	close(connected_socket->fd);
	free(connected_socket->message_buffer);
	free(connected_socket);
	return (start);
}

t_client *client_message(t_server *s, t_client *start, t_client *connected_socket)
{
	char message[1000000];
	char line[1000000];
	char buffer[1000100];
	int len;
	int i;
	int l;

	i = 0;
	l = 0;
	if ((len = recv(connected_socket->fd, message, sizeof(message) - 1, MSG_DONTWAIT)) == 0) //MSG_DONTWAIT enables nonblocking operation
		start = client_disconnection(s, start, connected_socket);
	else if (len == -1)
		return (start);
	else
	{
		message[len] = 0;
		while (message[i])
		{
			line[l] = message[i];
			if (message[i] == '\n')
			{
				line[l + 1] = 0;
				if (connected_socket->message_buffer != 0)
				{
					sprintf(buffer, "client %d: %s%s", connected_socket->id, connected_socket->message_buffer, line);
					free(connected_socket->message_buffer);
					connected_socket->message_buffer = 0;
				}
				else
					sprintf(buffer, "client %d: %s", connected_socket->id, line);
				send_to_all_clients(s, start, buffer, connected_socket->fd);
				l = -1;
			}
			l++;
			i++;
		}
		if (message[i - 1] != '\n')
		{
			line[l] = 0;
			if ((connected_socket->message_buffer = str_join(connected_socket->message_buffer, line)) == 0)
				fatal(6);
		}
	}
	return (start);
}

t_client *client_connection(t_server *s, t_client *c)
{
	int new_connection;
	char buffer[100];
	t_client *new;
	t_client *it;
	socklen_t len;

	it = c;
	len = sizeof(s->server_address);
	if ((new_connection = accept(s->server_socket, (struct sockaddr *)&s->server_address, &len)) == -1)
		return (c);
	if (new_connection > s->max_fds)
		s->max_fds = new_connection;
	FD_SET(new_connection, &s->sockets);
	if ((new = malloc(sizeof(t_client))) == 0) //sizeof(t_client *) created segfaults while sizeof(t_client) did not
		fatal(5);
	new->fd = new_connection;
	new->id = s->max_id++;
	new->next = 0;
	new->message_buffer = 0;
	if (c == 0)
		c = new;
	else
	{
		while (it->next != 0)
			it = it->next;
		it->next = new;
	}
	sprintf(buffer, "server: client %d just arrived\n", new->id);
	send_to_all_clients(s, c, buffer, new_connection);
	return (c);
}

void launch_server(t_server s)
{
	t_client *c;
	t_client *it;

	c = 0;
	s.max_id = 0;
	s.max_fds = s.server_socket;
	FD_ZERO(&s.sockets);
	FD_SET(s.server_socket, &s.sockets);
	while (1)
	{
		s.read_set = s.write_set = s.sockets;
		it = c;
		if (select(s.max_fds + 1, &s.read_set, &s.write_set, 0, 0) == -1)
			fatal(4);
		if (FD_ISSET(s.server_socket, &s.read_set))
		{
			c = client_connection(&s, c);
			continue ;
		}
		while (it)
		{
			if (FD_ISSET(it->fd, &s.read_set))
				c = client_message(&s, c, it);
			it = it->next;
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

	if (argc != 2)
	{
		write(2, "Wrong number of arguments\n", strlen("Wrong number of arguments\n"));
		exit(1);
	}
	setup_server(&s, atoi(argv[1]));
	launch_server(s);
	return (0);
}
