#ifndef HASHMAP_H
#define HASHMAP_H

#define START_SIZE 2
#define TRUE 1
#define FALSE 0
#define ERROR 1

typedef void (*handler_t)(int, ...);


typedef struct Node {
    int key;
    handler_t value;
    struct Node* next;
} Node;

typedef struct {
    Node** buckets;
    int size;
    int capacity;
} HashMap;

HashMap* hashmap_create(int capacity);
void hashmap_insert(HashMap* map, int key, handler_t value);
handler_t hashmap_get(HashMap* map, int key);
void hashmap_delete(HashMap* map, int key);
void hashmap_resize(HashMap* map, int new_capacity);

#endif /* HASHMAP_H */
