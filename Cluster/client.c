#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>

#define PORT 8080
#define BUFFER_SIZE 4096
#define AES_KEY "jasonjasonjasonjasonjasonjasonj"  // 32 bytes

int sock = 0;

void socketHandler(void);
void sendQuery(char* query);
char* readFile(char ruta[100]);
void sendParagraphsToMaster(char *texto);
char* cipherWithAES(const char *linea);
void handle_errors(void);

void socketHandler() {
    sock = 0;
    struct sockaddr_in server_addr;

    // Crear el socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Error al crear el socket");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    // Convertir la dirección IP del servidor a formato binario
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        perror("Dirección inválida o no soportada");
        exit(EXIT_FAILURE);
    }

    // Conectar al servidor
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error al conectar");
        exit(EXIT_FAILURE);
    }
}

void sendQuery(char* query) {
    char response[BUFFER_SIZE] = {0};
    send(sock, query, strlen(query), 0);
    
    read(sock, response, BUFFER_SIZE);
}

char* readFile(char ruta[100]) {
    FILE *archivo = fopen(ruta, "r");
    if (archivo == NULL) {
        perror("Error al abrir el archivo");
        return NULL;
    }

    // Mover el puntero de archivo al final para obtener el tamaño del archivo
    fseek(archivo, 0, SEEK_END);
    long tamano = ftell(archivo);
    fseek(archivo, 0, SEEK_SET);

    // Reservar memoria para almacenar el contenido del archivo
    char *contenido = (char*)malloc(tamano + 1);
    if (contenido == NULL) {
        perror("Error al asignar memoria");
        fclose(archivo);
        return NULL;
    }

    // Leer el contenido del archivo
    fread(contenido, sizeof(char), tamano, archivo);
    contenido[tamano] = '\0';

    fclose(archivo);

    return contenido;
}

void sendParagraphsToMaster(char *texto) {
    char *linea = strtok(texto, "\n");

    while (linea != NULL) {
        if (strlen(linea) > 0) {
            char *lineaCifrada = cipherWithAES(linea);
            sendQuery(lineaCifrada);
            free(lineaCifrada); // Liberar memoria del texto cifrado
        }
        linea = strtok(NULL, "\n");
    }

    char* endLine = "EOF\0";
    sendQuery(endLine);

    close(sock);
}

char* cipherWithAES(const char *linea) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    int len;
    int ciphertext_len;
    unsigned char ciphertext[BUFFER_SIZE]; 

    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_ecb(), NULL, (unsigned char *)AES_KEY, NULL)) {
        handle_errors();
    }

    if (1 != EVP_EncryptUpdate(ctx, ciphertext, &len, (unsigned char *)linea, strlen(linea))) {
        handle_errors();
    }
    ciphertext_len = len;

    if (1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len)) {
        handle_errors();
    }
    ciphertext_len += len;

    EVP_CIPHER_CTX_free(ctx);

    char *hex_ciphertext = malloc((ciphertext_len * 2) + 1);
    if (hex_ciphertext == NULL) {
        handle_errors();
    }

    for (int i = 0; i < ciphertext_len; i++) {
        sprintf(hex_ciphertext + (i * 2), "%02x", ciphertext[i]);
    }
    hex_ciphertext[ciphertext_len * 2] = '\0';

    return hex_ciphertext;
}

void handle_errors(void) {
    ERR_print_errors_fp(stderr);
    abort();
}

int main() {
    socketHandler();

    char ruta[100];
    printf("Ruta del archivo de texto: ");
    scanf("%s", ruta);

    char* texto = readFile(ruta);
    if (texto != NULL) {
        sendParagraphsToMaster(texto);
        free(texto);  // Liberar memoria después de procesar el archivo
    }

    return 0;
}

