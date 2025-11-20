#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <locale.h>
#include <windows.h>

#define MAX_CITIES 200
#define MAX_NAME_LEN 50
#define MAX_DESTINATIONS 12

typedef struct {
    int cityId;
    int distance;
    int previousCity;
} Path;

typedef struct {
    Path *paths;
    int size;
    int capacity;
} PriorityQueue;

typedef struct {
    char name[MAX_NAME_LEN];
    int numNeighbors;
    int neighborIds[MAX_CITIES];
    int distances[MAX_CITIES];
} City;

City cities[MAX_CITIES];
int numCities = 0;

// Function prototypes
PriorityQueue *createQueue(int capacity);
void enqueue(PriorityQueue *pq, int cityId, int distance, int previousCity);
Path dequeue(PriorityQueue *pq);
void freeQueue(PriorityQueue *pq);
void dijkstra(City *cities, int numCities, int startId, int destinationId, double *timeTaken, int *visitedCount);
void loadCitiesFromCSV(const char *filename);
int getCityId(char *cityName);

int main() {
    setlocale(LC_ALL, "");  // Enable UTF-8 encoding

    char startCity[MAX_NAME_LEN];
    char destinationCities[MAX_DESTINATIONS][MAX_NAME_LEN];

    loadCitiesFromCSV("europe.csv");

    printf("Enter start city: ");
    fgets(startCity, MAX_NAME_LEN, stdin);
    startCity[strcspn(startCity, "\n")] = 0;

    for (int i = 0; i < MAX_DESTINATIONS; i++) {
        printf("Enter destination city %d: ", i + 1);
        fgets(destinationCities[i], MAX_NAME_LEN, stdin);
        destinationCities[i][strcspn(destinationCities[i], "\n")] = 0;
    }

    int startId = getCityId(startCity);
    if (startId == -1) {
        printf("Error: Start city not found.\n");
        return 1;
    }

    for (int i = 0; i < MAX_DESTINATIONS; i++) {
        int destinationId = getCityId(destinationCities[i]);
        if (destinationId == -1) {
            printf("Error: Destination city %s not found.\n", destinationCities[i]);
            continue;
        }

        double timeTaken;
        int visitedCount;
        dijkstra(cities, numCities, startId, destinationId, &timeTaken, &visitedCount);

        printf("\nShortest path from %s to %s:\n", startCity, destinationCities[i]);
        printf("Time taken: %.6f ms\n", timeTaken * 1000);
        printf("Cities involved in search: %d\n\n", visitedCount);
    }

    return 0;
}

void dijkstra(City *cities, int numCities, int startId, int destinationId, double *timeTaken, int *visitedCount) {
    PriorityQueue *pq = createQueue(numCities);

    int distances[MAX_CITIES];
    int previous[MAX_CITIES];
    int visited[MAX_CITIES] = {0};

    *visitedCount = 0;

    for (int i = 0; i < numCities; i++) {
        distances[i] = INT_MAX;
        previous[i] = -1;
    }

    distances[startId] = 0;
    enqueue(pq, startId, 0, -1);

    LARGE_INTEGER frequency, start, end;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&start);

    while (pq->size > 0) {
        Path current = dequeue(pq);

        if (visited[current.cityId]) continue;
        visited[current.cityId] = 1;
        (*visitedCount)++;

        if (current.cityId == destinationId) break;

        for (int i = 0; i < cities[current.cityId].numNeighbors; i++) {
            int neighbor = cities[current.cityId].neighborIds[i];
            int travelTime = cities[current.cityId].distances[i];

            if (!visited[neighbor] && distances[current.cityId] + travelTime < distances[neighbor]) {
                distances[neighbor] = distances[current.cityId] + travelTime;
                previous[neighbor] = current.cityId;
                enqueue(pq, neighbor, distances[neighbor], current.cityId);
            }
        }
    }

    QueryPerformanceCounter(&end);
    *timeTaken = (double)(end.QuadPart - start.QuadPart) / frequency.QuadPart;

    freeQueue(pq);
}

PriorityQueue *createQueue(int capacity) {
    PriorityQueue *pq = malloc(sizeof(PriorityQueue));
    pq->paths = malloc(capacity * sizeof(Path));
    pq->size = 0;
    pq->capacity = capacity;
    return pq;
}

void enqueue(PriorityQueue *pq, int cityId, int distance, int previousCity) {
    pq->paths[pq->size].cityId = cityId;
    pq->paths[pq->size].distance = distance;
    pq->paths[pq->size].previousCity = previousCity;
    pq->size++;

    for (int i = pq->size - 1; i > 0 && pq->paths[i].distance < pq->paths[i - 1].distance; i--) {
        Path temp = pq->paths[i];
        pq->paths[i] = pq->paths[i - 1];
        pq->paths[i - 1] = temp;
    }
}

Path dequeue(PriorityQueue *pq) {
    Path top = pq->paths[0];

    for (int i = 0; i < pq->size - 1; i++) {
        pq->paths[i] = pq->paths[i + 1];
    }

    pq->size--;
    return top;
}

void freeQueue(PriorityQueue *pq) {
    free(pq->paths);
    free(pq);
}

void loadCitiesFromCSV(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open file");
        exit(1);
    }

    char line[256];
    char city1[MAX_NAME_LEN], city2[MAX_NAME_LEN];
    int distance;

    while (fgets(line, sizeof(line), file)) {
        sscanf(line, "%49[^,],%49[^,],%d", city1, city2, &distance);

        int id1 = getCityId(city1);
        if (id1 == -1) {
            id1 = numCities;
            strcpy(cities[numCities++].name, city1);
        }

        int id2 = getCityId(city2);
        if (id2 == -1) {
            id2 = numCities;
            strcpy(cities[numCities++].name, city2);
        }

        cities[id1].neighborIds[cities[id1].numNeighbors] = id2;
        cities[id1].distances[cities[id1].numNeighbors++] = distance;

        cities[id2].neighborIds[cities[id2].numNeighbors] = id1;
        cities[id2].distances[cities[id2].numNeighbors++] = distance;
    }

    fclose(file);
}

int getCityId(char *cityName) {
    for (int i = 0; i < numCities; i++) {
        if (strcmp(cities[i].name, cityName) == 0) {
            return i;
        }
    }
    return -1;
}


