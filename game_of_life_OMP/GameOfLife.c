/*
*Giorgos Kritikakis 3309
*/

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#define THREADS 4
int **world;
int  LINES;
int  COLUMNS;

int **tmp_array;


//int cordination[100][2];
int **cordination;
/*
*arxikopoiei ton pinaka cordination
*/
void InitCordinations();

/*
* Diabazei to arxeio kai arxikopoiei ton world
*/
void load_world_from_file(char* filename);

/*
* Desmeyw thn mnhmh gia toys pinakes
*/
void allocateMemory(int lines, int columns);

void initializeDead(int **array);
/*
*prints world
*/
void printWorld();
/*
*prints world in file
*/
void printWorldAt(char*);
/*
* briskei an to sygkekrimeno keli zei(1) h' exei pethanei
*/
int get_next_state(int x,int y  );
/*
* Ypologizei ton arithmo twn zwntanwn geitonwn
*/
int num_neighbors(int x,int y);

/*
* ypologizei thn epomenh genia
*/
void nextGen(int , int );
void next_generation( int lines , int columns);

int main(int argc , char** argv){
	//char *filename=(char*)malloc(51*sizeof(char));
	//printf("give the name of the file(must be less than fifty characters):");
	//scanf("%s" , filename);

	if(argc!=4){
		printf("give parameters:input file  , generations , output file name\n");
		exit(0);
	}
	load_world_from_file(argv[1]);

	int i ;
	tmp_array = (int**)malloc(COLUMNS*sizeof(int*));
 	if( tmp_array==NULL ){ fprintf(stderr , "malloc fail");exit(0);}
	for(i = 0;i<COLUMNS;++i){
        	tmp_array[i] = (int*)malloc( LINES*sizeof(int) );
       		if( tmp_array[i]==NULL ){ fprintf(stderr , "malloc fail");exit(0);}
 	}
	
	int generations = atoi(argv[2]);

	printf("generations:%d\n" , generations);
	while(generations>0){
		#pragma omp parallel
		{ 
		#pragma omp for
		for (i = 0; i < THREADS; ++i){
			int start , end;
			if(i!=THREADS-1){
				start = i*(LINES/THREADS); end = i*(LINES/THREADS)+LINES/THREADS;
			}else{
				start = i*(LINES/THREADS); end = LINES;
			}
			int k,j;
			for (k=start;k<end;k++){
				for(j=0;j<COLUMNS;j++){
				     tmp_array[k][j]=get_next_state(k,j);
   				}
 			}
		}
		#pragma omp for
                for (i = 0; i < THREADS; ++i){
			int start , end;
			if(i!=THREADS-1){
                                start = i*(LINES/THREADS); end = i*(LINES/THREADS)+LINES/THREADS;
                        }else{
                                start = i*(LINES/THREADS); end = LINES;
                        }
			int k,j;
			for (k=start;k<end;k++){
  				for(j=0;j<COLUMNS;j++){
  				   world[k][j]=tmp_array[k][j];
 				}
			}
		}
		}
		//nextGen(0 , LINES);
		--generations;
	}
	//printWorld();
	printWorldAt(argv[3]);

	return 1;
}


void load_world_from_file(char* filename){
 char c;
 int CellsTillNow=0;
 FILE *fp;
 fp = fopen(filename, "r");
 if(fp == (FILE *)NULL) {
   fprintf(stderr, "File '%s' does not exist program exiting\n",filename);
   exit(0);
 }

 /*
  * Desmeyse mnhmh gia thn metablhth world kai new world kai arxikopoihse
  */

 if(fscanf(fp, "%d", &LINES)==0 || fscanf(fp, "%d", &COLUMNS)==0 )
 {
   printf("input file has wrong format cannot initialize LINES and COLUMNS , exiting\n" );
   exit(0);
 }
 allocateMemory( LINES , COLUMNS);

 while((c = getc(fp)) != EOF ){
   if(c==' '){
     if(CellsTillNow<COLUMNS*LINES){
        world[(int)CellsTillNow/COLUMNS][CellsTillNow%COLUMNS]=0;
     }else{
	printf("input file has wrong format\n");
     }
     CellsTillNow++;
   }
   else if(c=='*'){
     if(CellsTillNow<COLUMNS*LINES){
         world[(int)CellsTillNow/COLUMNS][CellsTillNow%COLUMNS]=1;
     }else{
        printf("input file has wrong format\n");
     }
     CellsTillNow++;
   }
 }
 
if(CellsTillNow!=COLUMNS*LINES){
    printf("input file has wrong format %d %d , exiting\n"  ,CellsTillNow , COLUMNS*LINES);
    exit(0);
 }

 fclose(fp);
}

/*
* Desmeyw thn mnhmh gia toys pinakes
*/
void allocateMemory(int lines, int columns){
 int i,j;

 world=(int**)malloc(lines*sizeof(int*));
 for(i=0;i<lines;i++){
   world[i]=(int*)malloc(columns*sizeof(int));
 }

 initializeDead(world);
}


void initializeDead(int **array){
 int i,j;
 for(i=0;i<LINES;i++){
   for(j=0;j<COLUMNS;j++){
     array[i][j]=0;
   }
 }
}


/*
*prints world 
*/
void printWorld(){
 int i ,j;
 for(i=0;i<LINES;++i){
        for(j=0;j<COLUMNS;++j){
		printf("%d ", world[i][j]);
	}
	printf("\n");
 }
}

/*
*prints world at file
*/
void printWorldAt(char* filename){
   int i ,j , cell; 
   FILE *fp;
   fp = fopen(filename, "w"); 
   if (fp == (FILE *)NULL) { 
   	fprintf(stderr, "File '%s' does not exist program exiting\n", filename); 
	exit(0); 
   } 
   for(i=0;i<LINES;++i){
	fprintf(fp , "|");
	for(j=0;j<COLUMNS;++j){
		cell = world[i][j];
		if(cell==0)
			fprintf(fp," |");
		else
			fprintf(fp,"*|");
	}
       	fprintf(fp , "\n");
   }
   fclose(fp);
}


int get_next_state(int x,int y ) {
 int zwntanoiGeitones=num_neighbors(x,y);
 /* An to keli einai zwntano */
 if (world[x][y]==1){
   if(zwntanoiGeitones==2||zwntanoiGeitones==3){
     return 1;
   }
   else{
     return 0;
   }
 }
 /* An to keli einai nekro */
 else{
   if(zwntanoiGeitones==3){
     return 1;
   }
   else{
     return 0;
   }
 }
}


int num_neighbors(int x,int y){

 int zwntanoiGeitones=0;
 /* panw aristera */
 if (x-1>=0&&y-1>=0&&world[x-1][y-1]==1){
   zwntanoiGeitones++;
 }
 /* panw */
 if (x-1>=0&&world[x-1][y]==1){
   zwntanoiGeitones++;
 }
 /* panw deksia */
 if (x-1>=0&&y+1<COLUMNS&&world[x-1][y+1]==1){
   zwntanoiGeitones++;
 }
 /* deksia */
 if (y+1<COLUMNS&&world[x][y+1]==1){
   zwntanoiGeitones++;
 }
 /* aristera */
 if (y-1>=0&&world[x][y-1]==1){
   zwntanoiGeitones++;
 }
 /* katw aristera */
 if (x+1<LINES && y-1>=0&&world[x+1][y-1]==1){
   zwntanoiGeitones++;
 }
 /* katw */
 if (x+1<LINES&&world[x+1][y]==1){
   zwntanoiGeitones++;
 }
 /* katw deksia */
 if (x+1<LINES&&y+1<COLUMNS&&world[x+1][y+1]==1){
   zwntanoiGeitones++;
 }
 return zwntanoiGeitones;
}



/*
* ypologizei thn epomenh genia
*/
void nextGen(int start , int end){
 int k , j , i;

/* int i , **array = (int**)malloc(COLUMNS*sizeof(int*));
 if( array==NULL ){ fprintf(stderr , "malloc fail");exit(0);}
 for(i = 0;i<COLUMNS;++i){
	array[i] = (int*)malloc( (end-start)*sizeof(int) );
 	if( array[i]==NULL ){ fprintf(stderr , "malloc fail");exit(0);}
 }*/
 for (k=start;k<end;k++){
   for(j=0;j<COLUMNS;j++){
     tmp_array[k][j]=get_next_state(k,j);
   }
 }
// #pragma omp barrier
 for (i=start;i<end;i++){
   for(j=0;j<COLUMNS;j++){
     world[i][j]=tmp_array[i][j];
   }
 }

}


