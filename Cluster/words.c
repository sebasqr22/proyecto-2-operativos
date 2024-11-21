#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <ctype.h>

#define PORT 8080
#define MAX_PARAGRAPH_SIZE 4096
#define AES_KEY_LENGTH 32
#define TOP_WORDS 5

unsigned char aes_key[AES_KEY_LENGTH] = "jasonjasonjasonjasonjasonjasonj";

// Estructura de nodo para la lista enlazada de párrafos
typedef struct ParagraphNode {
    char paragraph[MAX_PARAGRAPH_SIZE];
    struct ParagraphNode *next;
} ParagraphNode;

// Estructura para contar palabras
typedef struct {
    char word[50];
    int count;
} WordCount;

// Variables globales
int server_fd;
struct sockaddr_in address;
ParagraphNode *head = NULL; // Cabeza de la lista enlazada

// Prototipos
void socketHandler();
void acceptQueries();
void insertParagraph(char *paragraph);
void printAndFreeList();

// Maneja la configuración del socket
void socketHandler(){
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Error al crear el socket");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Error al hacer el bind");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("Error al escuchar");
        exit(EXIT_FAILURE);
    }

    printf("Esperando conexiones en el puerto %d...\n", PORT);
}

// Acepta las queries del cliente y las almacena en la lista enlazada
void acceptQueries(){
    int new_socket;
    int addrlen = sizeof(address);
    char query[MAX_PARAGRAPH_SIZE] = {0};
    char response[10] = "Received\0";

    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        perror("Error al aceptar la conexión");
        exit(EXIT_FAILURE);
    }

    while (1) {
        memset(query, 0, MAX_PARAGRAPH_SIZE);
        read(new_socket, query, MAX_PARAGRAPH_SIZE);

        // Verificar si se recibió "EOF"
        if (strcmp(query, "EOF") == 0) {
            break;
        }

        // Insertar el párrafo en la lista enlazada
        insertParagraph(query);
        
        // Enviar una respuesta al cliente
        send(new_socket, response, strlen(response), 0);
    }

    close(new_socket);
    close(server_fd);
}

// Inserta un nuevo párrafo en la lista enlazada
void insertParagraph(char *paragraph) {
    ParagraphNode *newNode = (ParagraphNode *)malloc(sizeof(ParagraphNode));
    if (newNode == NULL) {
        perror("Error al asignar memoria para el nodo");
        exit(EXIT_FAILURE);
    }
    
    strncpy(newNode->paragraph, paragraph, MAX_PARAGRAPH_SIZE - 1);
    newNode->paragraph[MAX_PARAGRAPH_SIZE - 1] = '\0'; // Asegurar terminación de cadena
    newNode->next = NULL;

    if (head == NULL) {
        head = newNode;
    } else {
        ParagraphNode *current = head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = newNode;
    }
}

// Función para descifrar AES-256-ECB
int aes_decrypt_ecb(const unsigned char *ciphertext, int ciphertext_len, unsigned char *key, unsigned char *plaintext) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    int len, plaintext_len;

    EVP_DecryptInit_ex(ctx, EVP_aes_256_ecb(), NULL, key, NULL);
    EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len);
    plaintext_len = len;
    EVP_DecryptFinal_ex(ctx, plaintext + len, &len);
    plaintext_len += len;

    EVP_CIPHER_CTX_free(ctx);
    return plaintext_len;
}

// Función para convertir de hexadecimal a bytes
void hex_to_bytes(const char *hex, unsigned char *bytes, int bytes_len) {
    for (int i = 0; i < bytes_len; i++) {
        sscanf(hex + 2 * i, "%02hhx", &bytes[i]);
    }
}

// Función para convertir a minúsculas
void to_lowercase(char *str) {
    for (int i = 0; str[i]; i++) {
        str[i] = tolower((unsigned char) str[i]);
    }
}

// Función para contar palabras
void count_words(char *text, WordCount *word_counts, int *word_count) {
    char *word = strtok(text, " ,.-\n");

    while (word != NULL) {
        to_lowercase(word);
        int found = 0;

        for (int i = 0; i < *word_count; i++) {
            if (strcmp(word_counts[i].word, word) == 0) {
                word_counts[i].count++;
                found = 1;
                break;
            }
        }

        if (!found && *word_count < 1000) {
            strcpy(word_counts[*word_count].word, word);
            word_counts[*word_count].count = 1;
            (*word_count)++;
        }

        word = strtok(NULL, " ,.-\n");
    }
}

// Función para obtener las 5 palabras más comunes
void get_top_words(WordCount *word_counts, int word_count, WordCount *top_words) {
    for (int i = 0; i < TOP_WORDS; i++) {
        int max_idx = i;
        for (int j = i + 1; j < word_count; j++) {
            if (word_counts[j].count > word_counts[max_idx].count) {
                max_idx = j;
            }
        }

        WordCount temp = word_counts[i];
        word_counts[i] = word_counts[max_idx];
        word_counts[max_idx] = temp;

        top_words[i] = word_counts[i];
    }
}

// Función para combinar las palabras y sus conteos
void combine_word_counts(WordCount *combined, int *combined_count, WordCount *incoming, int incoming_count) {
    for (int i = 0; i < incoming_count; i++) {
        int found = 0;
        for (int j = 0; j < *combined_count; j++) {
            if (strcmp(combined[j].word, incoming[i].word) == 0) {
                combined[j].count += incoming[i].count;
                found = 1;
                break;
            }
        }
        if (!found && *combined_count < 1000) {
            combined[*combined_count] = incoming[i];
            (*combined_count)++;
        }
    }
}

// Función para liberar la memoria de la lista enlazada
void free_paragraph_list(ParagraphNode *head) {
    while (head != NULL) {
        ParagraphNode *temp = head;
        head = head->next;
        free(temp);
    }
}

int main(int argc, char *argv[]) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {  // Nodo maestro
        socketHandler();
        acceptQueries();

        // Distribuir los párrafos entre los nodos
        ParagraphNode *current = head;
        int paragraph_count = 0;
        while (current != NULL) {
            paragraph_count++;
            current = current->next;
        }

        int paragraphs_per_node = paragraph_count / size;
        int remaining_paragraphs = paragraph_count % size;

        current = head;
        for (int i = 1; i < size; i++) {
            int num_paragraphs = paragraphs_per_node + (i <= remaining_paragraphs ? 1 : 0);
            MPI_Send(&num_paragraphs, 1, MPI_INT, i, 0, MPI_COMM_WORLD);

            for (int j = 0; j < num_paragraphs; j++) {
                MPI_Send(current->paragraph, MAX_PARAGRAPH_SIZE, MPI_CHAR, i, 0, MPI_COMM_WORLD);
                current = current->next;
            }
        }

        WordCount word_counts[1000] = {0};
        int word_count = 0;

        // Procesar párrafos del nodo maestro
        for (int i = 0; current != NULL; i++) {
            int ciphertext_len = strlen(current->paragraph) / 2;
            unsigned char ciphertext[MAX_PARAGRAPH_SIZE];
            unsigned char decrypted[MAX_PARAGRAPH_SIZE];

            hex_to_bytes(current->paragraph, ciphertext, ciphertext_len);
            int decrypted_len = aes_decrypt_ecb(ciphertext, ciphertext_len, aes_key, decrypted);
            decrypted[decrypted_len] = '\0';

            count_words(decrypted, word_counts, &word_count);

            current = current->next;
        }

        // Obtener las 5 palabras más repetidas del nodo maestro
        WordCount top_words[TOP_WORDS];
        get_top_words(word_counts, word_count, top_words);

        // Recibir las listas de palabras de los nodos esclavos y combinarlas
        for (int i = 1; i < size; i++) {
            WordCount slave_top_words[TOP_WORDS];
            MPI_Recv(slave_top_words, TOP_WORDS * sizeof(WordCount), MPI_CHAR, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            combine_word_counts(word_counts, &word_count, slave_top_words, TOP_WORDS);
        }

        // Encontrar la palabra más repetida en todos los nodos
        get_top_words(word_counts, word_count, top_words);
        printf("\nLa palabra más repetida en total es: '%s' con %d repeticiones\n", top_words[0].word, top_words[0].count);

        // Liberar la lista enlazada
        free_paragraph_list(head);

    } else {  // Nodos esclavos

        printf("Nodo esclavo activo\n");
        int num_paragraphs;
        MPI_Recv(&num_paragraphs, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        WordCount word_counts[1000] = {0};
        int word_count = 0;
        char paragraph[MAX_PARAGRAPH_SIZE];

        // Procesar los párrafos en los nodos esclavos
        for (int i = 0; i < num_paragraphs; i++) {
            MPI_Recv(paragraph, MAX_PARAGRAPH_SIZE, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            int ciphertext_len = strlen(paragraph) / 2;
            unsigned char ciphertext[MAX_PARAGRAPH_SIZE];
            unsigned char decrypted[MAX_PARAGRAPH_SIZE];

            hex_to_bytes(paragraph, ciphertext, ciphertext_len);
            int decrypted_len = aes_decrypt_ecb(ciphertext, ciphertext_len, aes_key, decrypted);
            decrypted[decrypted_len] = '\0';

            count_words(decrypted, word_counts, &word_count);
        }

        // Obtener las 5 palabras más repetidas en este nodo
        WordCount top_words[TOP_WORDS];
        get_top_words(word_counts, word_count, top_words);

        // Enviar las palabras más frecuentes al nodo maestro
        MPI_Send(top_words, TOP_WORDS * sizeof(WordCount), MPI_CHAR, 0, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}
