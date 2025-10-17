/*
CS 420 
Assignment 2: CPU Scheduling
5
1
OSs Tested on: Ubuntu
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_PROC 100

typedef struct Process {
    int aTime;
    int pNum;
    int CPUBurst;
    int OGBurst;
    int doneTime;
    int priority;
} Process;

typedef struct Algorithm {
    char type[100];
    int timeQuantum;
    int numProc;
    struct Process process[MAX_PROC];
} Algorithm;

typedef struct Node {
    struct Process* processN;
    struct Node* next;
}Node;

typedef struct Q{
    int items[MAX_PROC];
    Node* front;
    Node* rear;
} Q;

struct Q* createQ(){
    Q* q = (Q*)malloc(sizeof( Q));

    if(q==NULL){exit(1);}
    q->rear = NULL;
    q->front = NULL;
    return q;
}

void enQ (Q* q, Process* a){
    Node* node = (Node*)malloc(sizeof(Node));
    if(node == NULL){exit(1);}
    node->processN = a;
    node->next = NULL;

    if(q->rear != NULL){
        q->rear->next = node;
        q->rear = node;
    }else{
        q->rear = node;
        q->front = node;
    }
}

int isEmpty(Q* q){return q->front == NULL;}

struct Process* deQ(Q* q){
    if(q->front == NULL){exit(1);}
    Node* temp = q->front;
    struct Process* a = temp->processN;

    q->front = q->front->next;

    if(q->front == NULL){q->rear = NULL;}

    free(temp);
    return a;
}

int isIn(Q* q, Process* a){
    int in = 0;
    if(q->front ==NULL){in = 0;}
    Node* temp = q->front;
    while(temp != NULL){
        if(temp->processN->pNum == a->pNum){
            in = 1;
            break;
        }
        temp = temp->next;
    }
    return in;
}

double calcTime(struct Algorithm* a){
    int totWait = 0;
    int turnaround = 0;
    int wait = 0;
    double avgW = 0;


    for (int i = 0; i<a->numProc;i++){
        turnaround = a->process[i].doneTime - a->process[i].aTime;
        wait = turnaround - a->process[i].OGBurst;
        totWait += wait;
    }

    avgW = (double)totWait / a->numProc;

    return avgW;
}

// Round Robin (RR)
void RR(struct Algorithm *rr) {

    int temp = rr->numProc;
    int time = 0;
    double avgW = 0;
    FILE* fptr = fopen("output.txt", "w");
    Q* FIFO = createQ();

    if (fptr == NULL) {return;}
    fprintf(fptr, "%s %d\n", rr->type, rr->timeQuantum);
    for(int i = 0; i < rr->numProc; i++) {
        rr->process[i].OGBurst = rr->process[i].CPUBurst;
        rr->process[i].doneTime = 0;
    }

    while(temp > 0){
        for(int j = 0; j < rr->numProc; j++) {
           if(rr->process[j].aTime <= time && rr->process[j].CPUBurst > 0 && !isIn(FIFO, &rr->process[j])) {
               enQ(FIFO, &rr->process[j]);
           }
        }

        if(!isEmpty(FIFO)){
            Process* currProc = deQ(FIFO);
            if (currProc->CPUBurst <= 0) {
                continue;
            }

            fprintf(fptr, "%d  %d\n", time, currProc->pNum);

            int eTime = (currProc->CPUBurst < rr->timeQuantum) ? currProc->CPUBurst : rr->timeQuantum;
            currProc->CPUBurst -= eTime;
            time += eTime;
            for(int j = 0; j < rr->numProc; j++) {
                if(rr->process[j].aTime < time && rr->process[j].CPUBurst > 0 && !isIn(FIFO, &rr->process[j])) {
                    enQ(FIFO, &rr->process[j]);
                }
            }

            if(currProc->CPUBurst <= 0) {
                currProc->doneTime = time;
                temp--;
            } else {
                enQ(FIFO, currProc);
            }
        } else {
            time++;
        }
    }

    avgW = calcTime(rr);

    printf("AVG Waiting Time: %.2f\n", avgW);
    fprintf(fptr, "AVG Waiting Time: %.2f\n\n", avgW);

    fclose(fptr);

}


// Shortest Job First (SJF)
void SJF(struct Algorithm *sjf) {

    int time = 0;
    int temp = sjf->numProc;
    double avgW = 0;
    Q* ready = createQ();
    Process *currProc = NULL;
    FILE* fptr = fopen("output.txt", "a");

    if(fptr == NULL){return;}

    //int shortProc = 0;
    //struct Process procInTime[sjf->procNum];

    fprintf(fptr,"%s\n", sjf->type);

    for(int i = 0; i<sjf->numProc;i++){
        sjf->process[i].OGBurst = sjf->process[i].CPUBurst;
        sjf->process[i].doneTime = 0;
    }

    while(temp > 0){
        for(int i = 0; i<sjf->numProc; i++){
            if(sjf->process[i].aTime <= time && sjf->process[i].CPUBurst > 0 && !isIn(ready, &sjf->process[i])){
                enQ(ready, &sjf->process[i]);
            }
        }

        if(currProc == NULL && !isEmpty(ready)){
            Node *n = ready->front;
            Process *shortest = n->processN;
            Node *preToShort = NULL;
            Node *pre = n;

            while(n != NULL){
                if(n->processN->CPUBurst < shortest->CPUBurst){
                    shortest = n->processN;
                    preToShort = pre;
                }
                pre = n;
                n = n->next;
            }

            if(preToShort == NULL){
                currProc = deQ(ready);
            } else {
                Node* shortN = preToShort->next;
                currProc = shortN->processN;
                preToShort->next = shortN->next;
                if(shortN->next == NULL){
                    ready->rear = preToShort;
                }
                free(shortN);
            }
        }
        if(currProc != NULL){
            fprintf(fptr, "%d  %d\n", time, currProc->pNum);

            time += currProc->CPUBurst;
            currProc->doneTime = time;
            currProc->CPUBurst = 0;
            temp--;
            currProc = NULL;
        }else{
            time++; //Avoid infinite wait by no new arrivals in time and empty list
        }
    }

    avgW= calcTime(sjf);
    fprintf(fptr, "AVG Waiting Time %.2f\n\n", avgW);
    fclose(fptr);
}

// Priority Scheduling without Preemption (PR_noPREMP)
void PR_noPREMP(struct Algorithm *no) {
    int hiPR = 1;
    int hiPRIndex;
    int time = 0;
    int avgW = 0;
    FILE *fptr1;
    fptr1 = fopen("output.txt", "a");

    if (fptr1 == NULL) {return;}

    fprintf(fptr1, "\n%s\n", no->type);
    for (int i = 0; i < no->numProc; i++) {
        for (int j = 0; j < no->numProc; j++) {
            if (no->process[j].priority == hiPR) {
                hiPRIndex = j;
            }
        }
        fprintf(fptr1, "%d %d\n", time, no->process[hiPRIndex].pNum);
        hiPR++;
        avgW = avgW + time;
        time = time + no->process[hiPRIndex].CPUBurst;
        no->process[hiPRIndex].CPUBurst = 0;
    }
    fprintf(fptr1, "AVG Waiting Time: %.1f\n\n", (float)avgW/no->numProc);
    fclose(fptr1);

}

// Priority Scheduling with Preemption (PR_withPREMP)
void PR_withPREMP(struct Algorithm *with) {
    int time = 0;
    int temp = with->numProc;
    double avgW;
    Q* ready = createQ();
    Process* currProc = NULL;
    FILE* fptr = fopen("output.txt", "a");

    if (fptr == NULL) {return;}

    printf("%s \n", with->type);
    fprintf(fptr, "%s \n", with->type);

    for (int i = 0; i < with->numProc; i++) {
        with->process[i].OGBurst = with->process[i].CPUBurst;
        with->process[i].doneTime = 0;
    }

    while (temp > 0) {
        for (int i = 0; i < with->numProc; i++) {
            if (&with->process[i] == currProc) { 
                continue; 
            }
            
            if (with->process[i].aTime <= time && with->process[i].CPUBurst > 0 && !isIn(ready, &with->process[i])) {
                enQ(ready, &with->process[i]);
            }
        }
        
        Process* prevCurrProc = currProc;
        Process* nextProc = currProc;
        Process* highestPrioProc = NULL;
        Node* highestPrioNode = NULL;
        Node* prevToHighest = NULL;
        
        if (!isEmpty(ready)) {
            Node* n = ready->front;
            Node* prev = NULL;
            highestPrioProc = n->processN;
            highestPrioNode = n;

            while (n != NULL) {
                if (n->processN->priority < highestPrioProc->priority) {
                    highestPrioProc = n->processN;
                    highestPrioNode = n;
                    prevToHighest = prev; 
                }
                prev = n; 
                n = n->next;
            }

            if (currProc == NULL) {
                nextProc = highestPrioProc;
            } 
            else if (highestPrioProc->priority < currProc->priority) {
                enQ(ready, currProc);
                nextProc = highestPrioProc;
            }

        }

        if (nextProc != currProc && nextProc != NULL) {
            if (highestPrioNode == ready->front) {
                deQ(ready);
            } else {
                prevToHighest->next = highestPrioNode->next;

                if (highestPrioNode == ready->rear) {
                    ready->rear = prevToHighest;
                }
                free(highestPrioNode);
            }
            currProc = nextProc;
        } 
        else if (currProc == NULL && nextProc != NULL) {
            if (highestPrioNode == ready->front) {
                deQ(ready);
            } else {
                prevToHighest->next = highestPrioNode->next;
                if (highestPrioNode == ready->rear) {
                    ready->rear = prevToHighest;
                }
                free(highestPrioNode);
            }
            currProc = nextProc;
        }

        if (currProc != prevCurrProc && currProc != NULL) {
            fprintf(fptr, "%d  %d\n", time, currProc->pNum);
        }
        if (currProc != NULL) {
            currProc->CPUBurst--;
            if (currProc->CPUBurst <= 0) {
                currProc->doneTime = time + 1;
                temp--;
                currProc = NULL;
            }
        }
        time++;
    }

    avgW = calcTime(with);
    fprintf(fptr, "AVG Waiting Time: %.2f\n\n", avgW);
    fclose(fptr);
}




int main() {
    struct Algorithm *algorithms = (struct Algorithm *)malloc(MAX_PROC*sizeof(struct Algorithm));
    int i = 0; //Index tracker of algorithms
    int endCheck;
    int temp1;

    //Make the file pointer
    FILE *fptr;
    char buffer[4096];
    fptr = fopen("input.txt", "r");
    if (fptr == NULL) {
        printf("Error! Could not open file\n");
        exit(1);
    }

    //sort through file data, moving to structs
    while(fscanf(fptr,"%d",&endCheck)!=EOF){

        fscanf(fptr, "%s %d", algorithms[i].type, &temp1);
        if(strcmp(algorithms[i].type, "RR")!=0){
           algorithms[i].timeQuantum = 0;
           algorithms[i].numProc = temp1;
        } else {
           algorithms[i].timeQuantum = temp1;
           fscanf(fptr, "%d", &algorithms[i].numProc);
        }
        for(int j =0; j<algorithms[i].numProc;j++){
            fscanf(fptr, "%d %d %d %d", &algorithms[i].process[j].pNum, &algorithms[i].process[j].aTime, &algorithms[i].process[j].CPUBurst, &algorithms[i].process[j].priority);
        }

        i++;
    }


    for(int k=0; k<i; k++){
        if(strcmp(algorithms[k].type, "RR")==0){
            RR(&algorithms[k]);
        } else if(strcmp(algorithms[k].type, "SJF")==0){
            SJF(&algorithms[k]);
        } else if(strcmp(algorithms[k].type, "PR_noPREMP")==0){
            PR_noPREMP(&algorithms[k]);
        } else if(strcmp(algorithms[k].type, "PR_withPREMP")==0){
            PR_withPREMP(&algorithms[k]);
        }
    }

    fclose(fptr);
    return 0;
}
