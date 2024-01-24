#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

// Définition de la structure header qui représente un bloc de mémoire libre
typedef struct header {
    size_t size;    // Taille du bloc de mémoire
    struct header* next;  // Pointeur vers le bloc de mémoire suivant
} Header;

// Pointeur vers le bloc de mémoire actuel
static Header* current_block = NULL;

// Pointeur vers la tête de la liste des blocs de mémoire libres
static Header* free_list = NULL;

// Pointeur vers la fin de la heap
static char* heap_end = NULL;

// Fonction pour allouer de la mémoire
void* my_malloc(size_t n);

// Fonction pour libérer de la mémoire
void my_free(void* p);

// Fonction pour réallouer de la mémoire
void* my_realloc(void* p, size_t n);

// Fonction pour allouer de la mémoire et l'initialiser à zéro
void* my_calloc(size_t nmemb, size_t size);

// Fonction pour allouer de la mémoire
void* my_malloc(size_t n) {
    Header* prev;  // Pointeur vers le bloc de mémoire précédent
    Header* v;     // Pointeur vers un nouveau bloc de mémoire
    size_t curr_size;  // Taille du bloc de mémoire actuel

    // Si la liste des blocs de mémoire libres est vide
    if (free_list == NULL) {
        // Allocation d'une nouvelle heap de 4096 octets avec sbrk
        if ((heap_end = sbrk(4096)) == (char*)-1) {
            return NULL;  // Échec de l'allocation
        }
        // Initialisation du bloc de mémoire actuel comme la tête de la heap
        current_block = (Header*)heap_end;
        current_block->size = 4096 - sizeof(Header);
        // Initialisation de la liste des blocs de mémoire libres
        free_list = current_block + 1;
        free_list->size = 0;
        current_block->next = NULL;  // Initialisation du pointeur next pour la tête
    }

    // Récupération de la taille du bloc de mémoire actuel
    curr_size = current_block->size;

    // Si la taille demandée est inférieure ou égale à la taille du bloc actuel
    if (n <= curr_size) {
        // Si la taille demandée est inférieure à la taille du bloc actuel moins la taille d'un header
        if (n < curr_size - sizeof(Header)) {
            prev = current_block;
            // Calcul du pointeur vers le nouveau bloc de mémoire libre
            v = (Header*)((char*)current_block + sizeof(Header) + n);
            v->size = curr_size - sizeof(Header) - n;
            v->next = current_block->next;
            current_block->size = n;
            current_block->next = v;
            free_list = prev;  // Ajout du bloc précédent à la liste des blocs libres
            return (char*)current_block + sizeof(Header);
        } else {
            current_block->size = n;
            return (char*)current_block + sizeof(Header);
        }
    } else if (curr_size == n) {
        // Si la taille demandée est égale à la taille du bloc actuel
        return (char*)current_block + sizeof(Header);
    } else {
        prev = current_block;
        while (curr_size < n && (char*)current_block + curr_size < heap_end) {
            current_block = current_block->next;
            // Si on atteint la fin de la heap sans avoir trouvé un bloc assez grand
            if (current_block == NULL) {
                // Allocation d'une nouvelle heap de 4096 octets avec sbrk
                if ((heap_end = sbrk(4096)) == (char*)-1) {
                    return NULL;
                }
                // Initialisation d'un nouveau bloc de mémoire actuel
                current_block = (Header*)((char*)heap_end - curr_size);
                current_block->size = 4096 - sizeof(Header);
                current_block->next = NULL;
                free_list = prev;
                // Appel récursif pour tenter à nouveau d'allouer la mémoire
                return my_malloc(n);
            }
            curr_size += current_block->size;
        }

        // Si on atteint la fin de la heap sans avoir trouvé un bloc assez grand
        if ((char*)current_block + curr_size == heap_end) {
            // Allocation d'une nouvelle heap de 4096 octets avec sbrk
            if ((heap_end = sbrk(4096)) == (char*)-1) {
                return NULL;
            }
            // Liaison du nouveau bloc de mémoire à la fin de la heap
            current_block->next = (Header*)((char*)current_block + curr_size);
            current_block = current_block->next;
            current_block->size = 4096 - sizeof(Header);
            current_block->next = NULL;
            free_list = prev;
            // Appel récursif pour tenter à nouveau d'allouer la mémoire
            return my_malloc(n);
        }

        // Si la taille demandée est inférieure à la somme de la taille du bloc actuel et de la taille du bloc suivant
        if (n < curr_size + current_block->size) {
            // Fusionner le bloc actuel avec le bloc suivant
            current_block->size += curr_size;
            return (char*)current_block + sizeof(Header);
        }

        // Si aucune condition n'est satisfaite, retourner NULL car on n'a pas pu allouer de mémoire
        return NULL;
    }
}

// Fonction pour libérer de la mémoire
void my_free(void* p) {
    Header* b;

    // Conversion du pointeur p en pointeur Header
    b = (Header*)p;
    b -= 1;
    b->next = free_list;
    free_list = b;
}

// Fonction pour réallouer de la mémoire

void* my_realloc(void* p, size_t n) {
    void* new_p;
    Header* b;
    size_t current_size;

    // Si la nouvelle taille est nulle
    if (n == 0) {
        my_free(p);
        return NULL;
    }

    // Si le pointeur p est NULL
    if (p == NULL) {
        return my_malloc(n);
    }

    // Conversion du pointeur p en pointeur Header
    b = (Header*)p - 1;

    // Calcul de la taille actuelle du bloc mémoire
    current_size = b->size;

    // Allocation d'une nouvelle zone mémoire de taille n
    new_p = my_malloc(n);
    if (new_p == NULL) {
        return NULL;
    }

    // Copie des données de l'ancienne zone mémoire vers la nouvelle
    memcpy(new_p, p, (n < current_size) ? n : current_size);

    // Libération de l'ancienne zone mémoire
    my_free(p);

    // On retourne le pointeur sur la nouvelle zone mémoire
    return new_p;
}

// Fonction pour allouer de la mémoire et l'initialiser à zéro
void* my_calloc(size_t nmemb, size_t size) {
    void* p;
    size_t total_size = nmemb * size;

    // Allocation d'une zone mémoire de taille total_size
    p = my_malloc(total_size);
    if (p != NULL) {
        // Initialisation de la zone mémoire à zéro
        memset(p, 0, total_size);
    }
    return p;
}

int main() {
    int *p, *newp, i;

    // Allocation d'un bloc de mémoire pour stocker 10 entiers avec my_malloc
    p = (int*) my_malloc(10 * sizeof(int));
    if (p != NULL) {
        printf("Mémoire allouée avec succès.\n");

        // Affichage du contenu initial du bloc de mémoire
        for (i = 0; i < 10; i++) {
            printf("p[%d] = %d\n", i, p[i]);
        }

        // Tentative de réallocation de mémoire pour 15 entiers avec my_malloc
        printf("Réallocation de mémoire pour 15 entiers...\n");
        newp = (int*) my_malloc(15 * sizeof(int));
        if (newp != NULL) {
            printf("Réallocation de mémoire réussie.\n");

            // Affichage du contenu du nouveau bloc de mémoire
            for (i = 0; i < 15; i++) {
                printf("newp[%d] = %d\n", i, newp[i]);
            }

            // Libération de la mémoire du nouveau bloc
            my_free(newp);
        } else {
            printf("Échec de la réallocation de mémoire.\n");
        }

        // Libération de la mémoire du bloc initial
        my_free(p);
    } else {
        printf("Échec de l'allocation de mémoire.\n");
    }

    return 0;
}