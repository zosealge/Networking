tcphost:
	gcc -g -o tcp_host tcp_serv.c -Wall -Wextra -Werror
tcpclient:
	gcc -g -o tcp_client tcp_client.c -Wall -Wextra -Werror
udphost:
	gcc -g -o udp_host udp_serv.c -Wall -Wextra -Werror
udpclient:
	gcc -g -o udp_client udp_client.c -Wall -Wextra -Werror