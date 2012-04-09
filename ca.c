#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <getopt.h>

#define FILENAME "lattice"
//#define DEBUG 1
#define TAG 0

/* vypocita nasledujucu interaciu */
void play(int *line,int *up_line,int *down_line,int cols,int rows,int myid,int *result_line);
/* vypise riadok na vystup */
void print_row(int *line,int length,int myid);
inline int calculate_cell(int *line,int *up_line,int *down_line,int cols,int i);
inline void send_value(int value);
void swap_pointers(int **a,int **b);

int myid;                   /* id procesora */
MPI_Status stat;            /* struct- obsahuje kod- source, tag, error */
int lines_count;            /* pocet riadkov */

int main(int argc, char *argv[]){

    int opt;                       /* parameter z prikazovej riadky */
    int cols;                      /* pocet stlpcov v matici*/
    FILE *f;                       /* subor */
    int *line;                     /* nacitany riadok pre procesor */
    int *up_line,*up_line_tmp;     /* predchadzajuci riadok */
    int *down_line,*down_line_tmp; /* nasledujuci riadok */
    int *result_line;              /* obsahuje vysledok po kazdej iteracii */
    int reapeat;                   /* pocet opakovani hry */
    int char_int;                  /* obsahuje nacitanu ciselnu hodnotu */
    int index_up = 0;
    int index_down = 0;
    int i = 0;
    int j = 0;

     /* z paremetru ziskaj pocet stlpcov */
    while((opt = getopt(argc, argv, "c:s:")) != -1) {
            if(opt == 'c'){
                cols = atoi(optarg);
            }
            if(opt == 's'){
                reapeat = atoi(optarg);
            }
    }

    /* MPI INIT */
    MPI_Init(&argc,&argv);                          /* inicializacia MPI */
    MPI_Comm_size(MPI_COMM_WORLD, &lines_count);    /* zisti kolko procesov bezi */
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);           /* zisti id svojho procesu */


    line = (int *)malloc(cols * sizeof(int));
    up_line = (int *)malloc(cols * sizeof(int));
    down_line = (int *)malloc(cols * sizeof(int));
    result_line = (int *)malloc(cols * sizeof(int));
    up_line_tmp = (int *)malloc(cols * sizeof(int));
    down_line_tmp = (int *)malloc(cols * sizeof(int));

    if(line == NULL || up_line == NULL || down_line == NULL ||
        result_line == NULL){
        fprintf(stderr,"line mallock error\n");
        return 1;
    }


    f = fopen(FILENAME,"r");
    fseek(f,(cols+1)*myid,SEEK_SET);/* zacni citat od myid riadka */

    int c;
    while((c = fgetc(f)) != '\n'){
        if(c == EOF) break;/* ak nahodou obsahuje koniec bez '\n' */

        char_int = c - '0';/* prevod char -> int */

        line[i++] = char_int;

        if(lines_count > 1){

             MPI_Request req;

            /* ak je posledny tak dalej neposielaj */
            if(myid != (lines_count - 1)){
                MPI_Isend(&char_int, 1, MPI_INT, (myid + 1), TAG, MPI_COMM_WORLD,&req);
            }
            /* ak je prvy tak neposiela na prechadzajuci */
            if(myid > 0){
                MPI_Isend(&char_int, 1, MPI_INT, (myid - 1), TAG, MPI_COMM_WORLD,&req);
            }

            /* obidve hodnoty su odoslane */


            /* prvy neprijima od predchadzajuceho */
            if(myid != 0){
                MPI_Recv(&up_line[index_up++],1,MPI_INT,myid-1,TAG,MPI_COMM_WORLD,&stat);
            }

            /* posledny neprjima od nasledujuceho */
            if(myid != (lines_count - 1)){
                MPI_Recv(&down_line[index_down++],1,MPI_INT,myid+1,TAG,MPI_COMM_WORLD,&stat);
            }

            MPI_Wait(&req,&stat);
        }
    };

    fclose(f);


    /* vypocet interacii hry */
    for(i = 0; i < reapeat; i++){
        for(j = 0; j < cols; j++){
            result_line[j] = calculate_cell(line,up_line,down_line,cols,j);

                MPI_Request req;

                /* ak je posledny tak dalej neposielaj */
                if(myid != (lines_count - 1)){
                    MPI_Isend(&result_line[j], 1, MPI_INT, (myid + 1), TAG, MPI_COMM_WORLD,&req);
                }
                /* ak je prvy tak neposiela na prechadzajuci */
                if(myid > 0){
                    MPI_Isend(&result_line[j], 1, MPI_INT, (myid - 1), TAG, MPI_COMM_WORLD,&req);
                }



            //send_value(result_line[j]);
            /* prvy neprijima od predchadzajuceho */
            if(myid != 0){
                MPI_Recv(&up_line_tmp[j],1,MPI_INT,myid-1,TAG,MPI_COMM_WORLD,&stat);
            }

            /* posledny neprjima od nasledujuceho */
            if(myid != (lines_count - 1)){
                MPI_Recv(&down_line_tmp[j],1,MPI_INT,myid+1,TAG,MPI_COMM_WORLD,&stat);
            }

            /* obidve hodnoty su odoslane */
                MPI_Wait(&req,&stat);
        }
        /*play(line,up_line,down_line,cols,lines_count,myid,result_line);*/
        //MPI_Barrier(MPI_COMM_WORLD);/* treba pockat na dopocitanie hodnot */
        /* v dalsom kole bude pocitat s vysledkom */
        swap_pointers(&result_line,&line);
        /* zamena tmp*/
        swap_pointers(&up_line,&up_line_tmp);
        swap_pointers(&down_line,&down_line_tmp);
    }

    print_row(line,cols,myid);

    MPI_Finalize();
    return 0;
}

void swap_pointers(int **a,int **b){
    int *tmp_line = *a;
    *a = *b;
    *b = tmp_line;
}

inline void send_value(int value){

    MPI_Request req;

    /* ak je posledny tak dalej neposielaj */
    if(myid != (lines_count - 1)){
        MPI_Isend(&value, 1, MPI_INT, (myid + 1), TAG, MPI_COMM_WORLD,&req);
    }
    /* ak je prvy tak neposiela na prechadzajuci */
    if(myid > 0){
        MPI_Isend(&value, 1, MPI_INT, (myid - 1), TAG, MPI_COMM_WORLD,&req);
    }

    /* obidve hodnoty su odoslane */
    MPI_Wait(&req,&stat);
}

inline int calculate_cell(int *line,int *up_line,int *down_line,int cols,int i){

        int value;
        int counter = 0;

        /* okrajove hodnoty matice */
        if( myid == 0 || i == 0 || myid == (lines_count - 1) || i == (cols - 1)) {

            /* je to prvy riadok alebo posledny riadok */
            if(myid == 0 || myid == (lines_count - 1)){
                /* a nie je to prvy stlpec a posledny stlpec */
                if(i != 0 && i != (cols - 1)){

                    if(myid == 0){/* prvy riadok ma len dolne */
                        if(down_line[i]) counter++;/*hodnota dole*/
                        if(down_line[i+1]) counter++;/*vpravo dole */
                        if(down_line[i-1]) counter++;/*vlavo dole */
                    }else{/* posledny len horne */
                        if(up_line[i]) counter++;/*hodnota hore*/
                        if(up_line[i+1]) counter++;/*vpravo hore */
                        if(up_line[i-1]) counter++;/*vlavo hore */
                    }

                    if(line[i+1]) counter++;/* vpravo */
                    if(line[i-1]) counter++;/* vlavo */
                }
            }

            if(i == 0 || i == (cols - 1)){/* je to prvy alebo posledny stlpec */

                /* a nie je to prvy riadok alebo posledny riadok */
                if(myid != 0 && myid != (lines_count - 1)){
                    /* prvy stlpec */
                    if(i == 0){
                        if(up_line[i+1]) counter++;/*vpravo hore */
                        if(down_line[i+1]) counter++;/*vpravo dole */
                        if(line[i+1]) counter++;/* vpravo */
                    }else{/* posledny stlpec */
                        if(down_line[i-1]) counter++;/*vlavo dole */
                        if(up_line[i-1]) counter++;/*vlavo hore */
                        if(line[i-1]) counter++;/* vlavo */
                    }
                    if(up_line[i]) counter++;/*hodnota hore*/
                    if(down_line[i]) counter++;/*hodnota dole*/
                }
            }

            /* prvy riadok prvy stlpec */
            if(myid == 0 && i == 0){
                if(down_line[i]) counter++;/*hodnota dole*/
                if(down_line[i+1]) counter++;/*vpravo dole */
                if(line[i+1]) counter++;/* vpravo */
            }

            /* prvy riadok posledny stlpec*/
            if(myid == 0 && i == (cols - 1)){
                if(line[i-1]) counter++;/* vlavo */
                if(down_line[i-1]) counter++;/*vlavo dole */
                if(down_line[i]) counter++;/*hodnota dole*/
            }

            /* posledny riadok prvy stlpec */
            if(myid == (lines_count - 1) && i == 0){
                if(up_line[i]) counter++;/*hodnota hore*/
                if(down_line[i+1]) counter++;/*vpravo dole */
                if(line[i+1]) counter++;/* vpravo */
            }

            /* posledny riadok posledny stlpec */
            if(myid == (lines_count - 1) && i == (cols - 1)){
                if(line[i-1]) counter++;/* vlavo */
                if(up_line[i-1]) counter++;/*vlavo hore */
                if(up_line[i]) counter++;/*hodnota hore*/
            }

        }else {/* stredne hodnoty matice bez okrajovych */

            if(down_line[i]) counter++;/*hodnota dole*/
            if(down_line[i+1]) counter++;/*vpravo dole */
            if(down_line[i-1]) counter++;/*vlavo dole */

            if(up_line[i]) counter++;/*hodnota hore*/
            if(up_line[i+1]) counter++;/*vpravo hore */
            if(up_line[i-1]) counter++;/*vlavo hore */

            if(line[i+1]) counter++;/* vpravo */
            if(line[i-1]) counter++;/* vlavo */

        }

        /* je ziva */
        if(line[i]){
            if(counter < 2) value = 0;/* Každá živá buňka s méně než dvěma živými sousedy umírá. */
            else if(counter == 2 || counter == 3) value = 1;/* Každá živá buňka se dvěma nebo třemi živými sousedy zůstává žít. */
            else if(counter > 3) value = 0;/* Každá živá buňka s více než třemi živými sousedy umírá. */
            else value = line[i];

        }else{/*mrtva*/
            if(counter == 3) value=1;/*Každá mrtvá buňka s právě třemi živými sousedy ožívá.*/
            else value = 0;
        }

        //printf("row:%d cell:%d value:%d counter:%d\n",myid,i,value,counter);

        return value;
}


void play(int *line,int *up_line,int *down_line,int cols,int rows, int myid,int *result_line){

    int i;
    int counter = 0;/*pocet zivich buniek*/

    /* prechadzaj cely riadok a nastav hodnotu (0|1)*/
    for(i = 0; i < cols; i++){

        /* okrajove hodnoty matice */
        if( myid == 0 || i == 0 || myid == (rows - 1) || i == (cols - 1)) {

            /* je to prvy riadok alebo posledny riadok */
            if(myid == 0 || myid == (rows - 1)){
                /* a nie je to prvy stlpec a posledny stlpec */
                if(i != 0 && i != (cols - 1)){

                    if(myid == 0){/* prvy riadok ma len dolne */
                        if(down_line[i]) counter++;/*hodnota dole*/
                        if(down_line[i+1]) counter++;/*vpravo dole */
                        if(down_line[i-1]) counter++;/*vlavo dole */
                    }else{/* posledny len horne */
                        if(up_line[i]) counter++;/*hodnota hore*/
                        if(up_line[i+1]) counter++;/*vpravo hore */
                        if(up_line[i-1]) counter++;/*vlavo hore */
                    }

                    if(line[i+1]) counter++;/* vpravo */
                    if(line[i-1]) counter++;/* vlavo */
                }
            }

            if(i == 0 || i == (cols - 1)){/* je to prvy alebo posledny stlpec */

                /* a nie je to prvy riadok alebo posledny riadok */
                if(myid != 0 && myid != (rows - 1)){
                    /* prvy stlpec */
                    if(i == 0){
                        if(up_line[i+1]) counter++;/*vpravo hore */
                        if(down_line[i+1]) counter++;/*vpravo dole */
                        if(line[i+1]) counter++;/* vpravo */
                    }else{/* posledny stlpec */
                        if(down_line[i-1]) counter++;/*vlavo dole */
                        if(up_line[i-1]) counter++;/*vlavo hore */
                        if(line[i-1]) counter++;/* vlavo */
                    }

                    if(up_line[i]) counter++;/*hodnota hore*/
                    if(down_line[i]) counter++;/*hodnota dole*/
                }
            }

            /* prvy riadok prvy stlpec */
            if(myid == 0 && i == 0){
                if(down_line[i]) counter++;/*hodnota dole*/
                if(down_line[i+1]) counter++;/*vpravo dole */
                if(line[i+1]) counter++;/* vpravo */
            }

            /* prvy riadok posledny stlpec*/
            if(myid == 0 && i == (cols - 1)){
                if(line[i-1]) counter++;/* vlavo */
                if(down_line[i-1]) counter++;/*vlavo dole */
                if(down_line[i]) counter++;/*hodnota dole*/
            }

            /* posledny riadok prvy stlpec */
            if(myid == (rows - 1) && i == 0){
                if(up_line[i]) counter++;/*hodnota hore*/
                if(down_line[i+1]) counter++;/*vpravo dole */
                if(line[i+1]) counter++;/* vpravo */
            }

            /* posledny riadok posledny stlpec */
            if(myid == (rows - 1) && i == (cols - 1)){
                if(line[i-1]) counter++;/* vlavo */
                if(up_line[i-1]) counter++;/*vlavo hore */
                if(up_line[i]) counter++;/*hodnota hore*/
            }

        }else {/* stredne hodnoty matice bez okrajovych */
            if(down_line[i]) counter++;/*hodnota dole*/
            if(down_line[i+1]) counter++;/*vpravo dole */
            if(down_line[i-1]) counter++;/*vlavo dole */

            if(up_line[i]) counter++;/*hodnota hore*/
            if(up_line[i+1]) counter++;/*vpravo hore */
            if(up_line[i-1]) counter++;/*vlavo hore */

            if(line[i+1]) counter++;/* vpravo */
            if(line[i-1]) counter++;/* vlavo */

        }



        /* je ziva */
        if(line[i]){
            if(counter < 2)result_line[i] = 0;/* Každá živá buňka s méně než dvěma živými sousedy umírá. */
            else if(counter == 2 || counter == 3)result_line[i]=1;/* Každá živá buňka se dvěma nebo třemi živými sousedy zůstává žít. */
            else if(counter > 3) result_line[i]=0;/* Každá živá buňka s více než třemi živými sousedy umírá. */
            else result_line[i]=line[i];

        }else{/*mrtva*/
            if(counter == 3) result_line[i]=1;/*Každá mrtvá buňka s právě třemi živými sousedy ožívá.*/
            else result_line[i]=line[i];
        }


        counter = 0;
    }



    return;
}

void print_row(int *line,int length,int myid){

    int i;

    printf("%d:",myid);
    for(i = 0; i < length; i++){
        printf("%d",line[i]);
    }
    printf("\n");

    return;
}
