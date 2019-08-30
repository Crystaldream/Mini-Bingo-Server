
#include<SFML/Network.hpp>
#include <iostream>
#include <chrono>
#include <thread>

using namespace sf;
using namespace std;

// Enviar evento ao jogador
bool sendEvent(string str, TcpSocket *client) {

	Packet packet_send;

	packet_send << str;

	// Verificar se o socket enviou o pacote com sucesso
	if (client->send(packet_send) != Socket::Done) {
		packet_send.clear();
		return false;
	}

	cout << str << endl;

	packet_send.clear();

	return true;

}

// Enviar cartao de numeros ao jogador
bool sendCard(vector<int> card, TcpSocket *client) {

	Packet packet_send;
	int size = card.size();
	
	// Adicionar numeros ao pacote
	for (int i = 0; i < size; i++) {
		packet_send << card[i];
	}

	// Verificar se o socket enviou o pacote com sucesso
	if (client->send(packet_send) != Socket::Done) {
		packet_send.clear();
		return false;
	}

	packet_send.clear();

	return true;

}

// Enviar numero ao jogador
bool sendNumber(int number, TcpSocket *client) {

	Packet packet_send;

	packet_send << number;

	// Verificar se o socket enviou o pacote com sucesso
	if (client->send(packet_send) != Socket::Done) {
		packet_send.clear();
		return false;
	}

	packet_send.clear();

	return true;

}

// Receber evento do jogador
string receiveEvent(TcpSocket *client) {

	Packet packet_receive;
	string str;

	//Check if socket->receive has sent the data successfully
	if (client->receive(packet_receive) != Socket::Done) {
		packet_receive.clear();
		return "";
	}

	packet_receive >> str;
	std::cout << str << endl;

	packet_receive.clear();

	return str;

}

// Receber evento de status yes ou no e total de numeros restantes
int receiveEventStatus(TcpSocket *client) {

	Packet packet_receive;

	string status = "";
	int leftValues = 0;

	// Verifica se o client->receive recebeu com sucesso um pacote
	if (client->receive(packet_receive) != Socket::Done) {
		packet_receive.clear();
		return -1;
	}

	packet_receive >> status >> leftValues;

	packet_receive.clear();

	return leftValues;

}

// Gerar cartao com 10 numeros aleatorios sem repeticao
vector<int> generateCardNumbers(int min, int max) {

	vector<int> card(0);

	// Seed
	srand((unsigned int)time(NULL));

	// Geracao de numeros aleatorios sem repeticao.
	for (int i = 0; i < 10; i++) {

		// Adicionar primeiro numero ao vector do cartao de numeros
		if (card.size() == 0) 
			card.push_back(rand() % (max - min + 1) + min);
		else if (card.size() > 0) {

			bool existe = false;
			int num;

			// A variavel existe serve como uma flag e marca o numero que acabou de ser gerado como ja existente
			// Se ja existir no vector, sai do ciclo e adiciona o numero apenas se a variavel existe ainda estiver a true
			while (!existe) {

				num = rand() % (max - min + 1) + min;
				existe = true;
				int size = card.size();

				for (int j = 0; j < i; j++) {

					if (card[j] == num) {
						existe = false;
						break;
					}

				}

			}

			// adicionar numero ao vector do cartao de numeros
			card.push_back(num);

		}

	}

	return card;

}

// Gerar um numero aleatorio
int generateRandomValue(int min, int max) {

	// Seed
	srand((unsigned int)time(NULL));

	return rand() % (max - min + 1) + min;

}


int main() {

	TcpListener listener;
	TcpSocket client;

	Packet packet_send;
	Packet packet_receive;
	
	std::cout << ".:: Server Online - Made by Telmo Reinas for Fabamaq ::. \n" << endl;

	// Escutar novas ligacoes no porto 2000
	if (listener.listen(2000) != Socket::Done) {
		std::cout << "Error while trying to listen to the port 2000 \n" << endl;
	}

	// Aceitar ligacao
	if (listener.accept(client) != Socket::Done) {
		std::cout << "Error establishing connection with client \n" << endl;
	}

	// Envio de eventos Connected e Jogo e tratamento de possiveis erros
	if (!sendEvent("Connected", &client)) {
		std::cout << "Error sending Event Connected \n" << endl;
		client.disconnect();
	}
	
	if (receiveEvent(&client).compare("Jogo") != 0) {
		std::cout << "Error receiving Event Jogo \n" << endl;
		client.disconnect();
	}

	// Gerar cartao de numeros aleatorios
	vector<int> card = generateCardNumbers(1, 50);

	// Enviar cartao e rececao da confirmacao
	if (!sendCard(card, &client)) {
		std::cout << "Error sending Event Send Card \n" << endl;
		client.disconnect();
	}

	if (receiveEvent(&client).compare("Received Card") != 0) {
		std::cout << "Error receiving card confirmation \n" << endl;
		client.disconnect();
	}

	// Total de numeros restantes para match
	int leftValues = 10;

	// Enviar numeros ao jogador ate que sejam todos certos
	while (true) {

		int number = generateRandomValue(1, 50);

		if (!sendNumber(number, &client)) {
			std::cout << "Error sending Event Send Number \n" << endl;
			client.disconnect();
			break;
		}
		
		// Esperar dois segundos ate enviar novo pedido
		std::this_thread::sleep_for(std::chrono::milliseconds(2000));

		leftValues = receiveEventStatus(&client);

		if (leftValues == 0)
			break;

	}

	// Todos os numeros certos, fim do jogo
	if (receiveEvent(&client).compare("Won") == 0)
		cout << "Game Won by Player - Game Over \n" << endl;

	client.disconnect();
	listener.close();

	return 0;

}