#include <stdio.h>
#include <stdlib.h>

#include "HashMap.h"

// Hash main_function to determine bucket index
int hash(int key, int size) {
    return key % size;
}

// Initialize a new hashmap with initial capacity
HashMap* hashmap_create(int size) {
    HashMap* map = malloc(sizeof(HashMap));
    map->buckets = calloc(size, sizeof(Node*));
    map->size = 0;
    map->capacity = size;
    return map;
}

// Insert a key-value pair into the hashmap
void hashmap_insert(HashMap* map, int key, handler_t value) {
    if (map->size >= map->capacity) {
        hashmap_resize(map, map->capacity * 2); // Double the capacity if needed same as fds
    }
    int index = hash(key, map->capacity);
    Node* newNode = malloc(sizeof(Node));
    newNode->key = key;
    newNode->value = value;
    newNode->next = map->buckets[index];
    map->buckets[index] = newNode;
    map->size++;
}

// Get the value associated with a key from the hashmap
handler_t hashmap_get(HashMap* map, int key) {
    int index = hash(key, map->capacity);
    Node* current = map->buckets[index];
    while (current != NULL) {
        if (current->key == key) {
            return current->value;
        }
        current = current->next;
    }
    return NULL; // Key not found
}

// Remove a key-value pair from the hashmap
void hashmap_delete(HashMap* map, int key) {
    int index = hash(key, map->capacity);
    Node* current = map->buckets[index];
    Node* previous = NULL;
    while (current != NULL) {
        if (current->key == key) {
            if (previous == NULL) {
                // Remove the first node in the linked list
                map->buckets[index] = current->next;
            } else {
                // Remove a node from the middle or end of the linked list
                previous->next = current->next;
            }
            free(current);
            map->size--;
            return;
        }
        previous = current;
        current = current->next;
    }
}

// Resize the hashmap to the specified capacity
void hashmap_resize(HashMap* map, int new_capacity) {
    HashMap* new_map = hashmap_create(new_capacity);
    for (int i = 0; i < map->capacity; i++) {
        Node* current = map->buckets[i];
        while (current != NULL) {
            hashmap_insert(new_map, current->key, current->value);
            current = current->next;
        }
    }
    // Update the map with the resized data
    free(map->buckets);
    map->buckets = new_map->buckets;
    map->size = new_map->size;
    map->capacity = new_map->capacity;
    free(new_map);
}


