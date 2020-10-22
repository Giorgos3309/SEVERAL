#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*
* Diabazei to arxeio kai arxikopoiei ton world
*/
void load_world_from_file(char* filename);
/* Kanei mono ths desmeysh ths mnhmhs */
void allocateMemory(int lines, int columns);
/*
* Prosthetei ston array pantoy ' '
*/
void initializeDead(int **array);
/*
* apothikeyei to apotelesma se arxeio
*/
void save_world_to_file(char* filename);
/*
* ypologizei thn epomenh genia
*/
void next_generation();
/*
* briskei an to sygkekrimeno keli zei(1) h' exei pethanei
*/
int get_next_state(int x,int y);
/*
* Ypologizei ton arithmo twn zwntanwn geitonwn
*/
int num_neighbors(int x,int y);
/*
* Antigrafei dyo pinakes
*/
void copyWorld(int **source,int **target);

int **world;
int **new_world;
int lines;
int columns;

int main(){
 char *FileName="input.txt",*FileWrite="output.txt";
 int i;
 load_world_from_file(FileName);
 for(i=0;i<50;i++){
   next_generation();
 }
 save_world_to_file(FileWrite);
 return 1;
}

void load_world_from_file(char* filename){
 char c,prwta[20];
 int CellsTillNow=0;
 FILE *fp;
 fp = fopen(filename, "r");
 if(fp == (FILE *)NULL) {
   fprintf(stderr, "File '%s' doesn???t exist program exiting\n",filename);
   exit(0);
 }
 
 /* Arxika diabazw apo to arxeio thn prwth grammh
  */
 fgets(prwta, 20, fp);
 sscanf (prwta, "%d %d",&lines,&columns);
 
 /*
  * Desmeyse mnhmh gia thn metablhth world kai new world kai arxikopoihse
  */
 allocateMemory(lines, columns);
 
 do{
   if(c==' '){
     CellsTillNow++;
   }
   else if(c=='*'){
     world[(int)CellsTillNow/columns][CellsTillNow%columns]=(int)'*';
     CellsTillNow++;
   }
 }
 while((c = getc(fp)) != EOF);
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

 new_world=(int**)malloc(lines*sizeof(int*));
 for(i=0;i<lines;i++){
   new_world[i]=(int*)malloc(columns*sizeof(int));
 }

 initializeDead(world);
 initializeDead(new_world);
}


void initializeDead(int **array){
 int i,j;
 for(i=0;i<lines;i++){
   for(j=0;j<columns;j++){
     array[i][j]=(int)' ';
   }
 }
}

/*
* Grafw sto arxeio kai diagrafw thn mnhmh
*/
void save_world_to_file(char* filename){
 FILE *fp;
 int i,j;
 fp = fopen(filename, "w");
 if(fp == (FILE *)NULL) {
   fprintf(stderr, "File '%s' doesn???t exist program exiting\n",filename);
   exit(0);
 }
 fprintf(fp, "%d %d\n", lines,columns);
 for(i = 0; i < lines; i++){
   for(j=0;j<columns;j++){
     fprintf(fp,"|%c",(char)world[i][j]);
   }
   fprintf(fp,"|\n");
 }
 fclose(fp);
 free(world);
 free(new_world);
}

void next_generation(){
 int i,j;
 for (i=0;i<lines;i++){
   for(j=0;j<columns;j++){
     new_world[i][j]=get_next_state(i,j);
   }
 }
 
 copyWorld(new_world,world);
 initializeDead(new_world);
}
int get_next_state(int x,int y) {
 int zwntanoiGeitones=num_neighbors(x,y);
 /* An to keli einai zwntano */
 if (world[x][y]=='*'){
   if(zwntanoiGeitones==2||zwntanoiGeitones==3){
     return (int)'*';
   }
   else{
     return (int)' ';
   }
 }
 /* An to keli einai nekro */
 else{
   if(zwntanoiGeitones==3){
     return (int)'*';
   }
   else{
     return (int)' ';
   }
 }
 

}
int num_neighbors(int x,int y){
 int zwntanoiGeitones=0;
 /* panw aristera */
 if (x-1>=0&&y-1>=0&&world[x-1][y-1]==(int)'*'){
   zwntanoiGeitones++;
 }
 /* panw */
 if (x-1>=0&&world[x-1][y]==(int)'*'){
   zwntanoiGeitones++;
 }
 /* panw deksia */
 if (x-1>=0&&y+1<columns&&world[x-1][y+1]==(int)'*'){
   zwntanoiGeitones++;
 }
 /* deksia */
 if (y+1<columns&&world[x][y+1]==(int)'*'){
   zwntanoiGeitones++;
 }
 /* aristera */
 if (y-1>=0&&world[x][y-1]==(int)'*'){
   zwntanoiGeitones++;
 }
 /* katw aristera */
 if (x+1<lines&&y-1>=0&&world[x+1][y-1]==(int)'*'){
   zwntanoiGeitones++;
 }
 /* katw */
 if (x+1<lines&&world[x+1][y]==(int)'*'){
   zwntanoiGeitones++;
 }
 /* katw deksia */
 if (x+1<lines&&y+1<columns&&world[x+1][y+1]==(int)'*'){
   zwntanoiGeitones++;
 }
 return zwntanoiGeitones;
}

/*
* Antigrafh toy pinaka world ston new_world kai antistrofa
*/
void copyWorld(int **source,int **target){
 int i,j;
 for(i=0;i<lines;i++){
   for(j=0;j<columns;j++){
     target[i][j]=source[i][j];
   }
 }
}
