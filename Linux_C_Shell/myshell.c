#include <errno.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

typedef struct Loc_Var{
	char *name;
	char *value;
	struct Loc_Var *next;
}LocVar;

LocVar *LocVar_table=NULL;

LocVar * findLocVar(char *name){
	if(!LocVar_table)return NULL;
        LocVar* ptr=LocVar_table;

	while( ptr!=NULL){
		if(strcmp(ptr->name , name)==0)break;
                ptr=ptr->next;
        }
        if(ptr==NULL){
                return NULL;
        }       	


	return ptr;
}

void push(char *name , char *value){
	int i=0;
	LocVar *ptr=LocVar_table;

	LocVar_table=(LocVar *)malloc( sizeof(LocVar) );
	
	LocVar_table->name=(char*)malloc( sizeof(char)*(strlen(name)+1) );
	strcpy(LocVar_table->name , name);

	LocVar_table->value=(char*)malloc( sizeof(char)*(strlen(value)+1) );
        strcpy(LocVar_table->value , value);


	LocVar_table->next=ptr;
}


void unset(char *name){
	if(LocVar_table==0)return;
	LocVar* prev=NULL , *ptr=LocVar_table;
	while( strcmp(ptr->name , name)!=0){
		prev=ptr;
		ptr=ptr->next;
	}
	if(ptr==NULL){
		printf("there is not local variable \"%s\"\n" , name);
		return ;
	}
	if(prev==NULL){
		prev=LocVar_table;
		LocVar_table=LocVar_table->next;
		//free(prev);
	}else{
		prev=ptr->next;
		//free(ptr);
	}
}

void printvars(){
	LocVar *ptr=LocVar_table;
	while(ptr!=NULL){
		printf("name:%s value:%s\n" , ptr->name , ptr->value );
		ptr=ptr->next;
	}
}


void type_prompt(){
 printf("%s@cs345sh/%s/$",getlogin() , get_current_dir_name()); // ??? getcwd()
}


char *no_spaces(char *str){
        assert(str);
        char *new_str=(char *)malloc(  sizeof(char)*(1+strlen(str)) );
        int i=0,j=0;
        while(str[i]!='\0'){
                if(str[i]!=' '){
                        new_str[j]=str[i];
                        ++j;
                }
                ++i;
        }
        new_str[j]='\0';
        return new_str;
}





char* read_command(){
	char *p_str;
	char c;
	int size=30*sizeof(char);
	int i=0;
	p_str=(char *)malloc(size);
 	if(p_str==NULL){
        	perror("malloc can not allocate space\n");
                return 0;
        }
	while(1 ){
                c=getc(stdin);
		if(c=='\t')
			c=' ';
                if(c=='\n')
                        break;
                p_str[i]=c;
        	++i;
		if(i==size-2){
			size+=size;
			p_str=(char *)realloc(p_str , size);
			if(p_str==NULL){
				perror("realloc can not allocate space\n");
				return 0;
			}
		}
        }
	p_str[i]='\0';
	return p_str;
}



char **get_tokens(char *command){
	int size=5 , i=0;
	char **tokens=(char **)malloc(size*sizeof(char*) );
	 if(tokens==NULL){
                perror("malloc can not allocate space\n");
                return 0;
        }
	if( (tokens[i]=strtok( command , " ") )!=NULL){
		++i;
		while((tokens[i] = strtok(NULL, " ")) != NULL){
			++i;
			if( i==size-2 ){
				size+=size;
				tokens=(char**)realloc( tokens  ,sizeof(char*)*size );
				if(tokens==NULL){
                               		perror("realloc can not allocate space\n");
                                	return 0;
                        	}
			}
		}
	}
	return tokens;
}



void DisplayTokens(char **tokens){
	assert(tokens);
	int i=0;
	while(tokens[i]!=NULL){
		printf("%d)%s\n" ,i , tokens[i]);
		++i;
	}
}


int isvalidName(char* loc_var_name){
	char *ptr=loc_var_name;
        int i=0;
        while(ptr[i]!='\0'){
                if( !isalpha(ptr[i])&&!isdigit(ptr[i])&&ptr[i]!='_')return 0;
                ++i;
        }
        return 1;
}

int main(){
	while(1){		
		char c , *cptr , *str , **tokens , **tokens1  , *command  , *output;
		int i=0 , cpid , status;
		int ispipe=0;
		int is_rdr_r=0;
		int is_rdr_l=0;
		int is_rdr_rr=0;
		int pipefd[2];

		type_prompt();
		str=read_command();if(str[0]=='\0')continue;
		tokens=get_tokens( strcpy( (char*)malloc( sizeof(char)*(strlen(str)+1) )  , str ));
		

		if( strcmp(tokens[0] , "exit" )==0){
			break ; 
		}else if(strcmp(tokens[0] , "cd" )==0){
			chdir(tokens[1]);continue;
		}else if(strcmp(tokens[0] , "set" )==0){
			if(tokens[1]==NULL )continue;
			int bool1=0;
			char *ptr1=NULL , *ptr2=NULL , *value=NULL , *var_name = strtok( strcpy( (char*)malloc( sizeof(char)*(strlen(tokens[1])+1) )  , tokens[1])  , "=");
			if ( !isvalidName(var_name) ){printf("set::not valid variable  name\n"); continue;} ;
			ptr1=str;

			if( (ptr1=strchr( str , '=' ))==NULL){printf("set::wrong format\n");continue ;}
			if( (ptr1=strchr( str , '\"')) ==NULL) { printf("set::wrong format\n");continue ;}
			
			++ptr1;
			ptr2=strchr( ptr1  , '\"' );
			if(ptr2==NULL)continue;
			value=(char*)malloc( (strlen(ptr1)+1)*sizeof(char) );
			
			int i=0;
			while(ptr1[i]!='\"' &&  ptr1[i]!='\0'){
				value[i]=ptr1[i];
				++i;
			}
			value[i]='\0';	
			push(var_name , value);
			
			continue;
		}else if( strcmp(tokens[0] , "unset" )==0){
			unset( tokens[1] );
			continue ;
		}else if( strcmp(tokens[0] , "printlvars" )==0){
			printvars();
			continue ;
		}else if( (cptr=strchr( str , '>' ))!=NULL || (cptr=strchr( str , '<' ))!=NULL ){
			if( *cptr=='>' && *(cptr+1)=='>' ){
				command= strtok( strcpy( (char*)malloc( sizeof(char)*(strlen(str)+1) ) , str ) , ">");
				is_rdr_rr=1;
				++cptr;
			}else if (*cptr=='>'){
				command= strtok( strcpy( (char*)malloc( sizeof(char)*(strlen(str)+1) ) , str ) , ">");
				is_rdr_r=1;
			}else if (*cptr=='<'){
				command= strtok( strcpy( (char*)malloc( sizeof(char)*(strlen(str)+1) ) , str ) , "<");
                                is_rdr_l=1;
			}
			
			if(*(++cptr)=='\0'){printf("redirection::wrong format\n");continue;}
			output= strtok( strcpy( (char*)malloc( sizeof(char)*(strlen(cptr)+1) ) , cptr ) , "/0");
			tokens=get_tokens( strcpy( (char*)malloc( sizeof(char)*(strlen(command)+1) )  , command ));
			
			
		}else if((cptr=strchr( str , '|' ))!=NULL ){
			if(*(cptr+1)=='\0'){perror("pipe format error\n");continue;}
			char *command1 , *command2  , **tokens2 ;
  			int pid;
			ispipe=1;
			command1= strtok( strcpy( (char*)malloc( sizeof(char)*(strlen(str)+1) ) , str ) , "|");
			cptr++;
			command2=strtok( strcpy( (char*)malloc( sizeof(char)*(strlen(cptr)+1) ) , cptr ) , "\0") ;
			
			tokens1=get_tokens( strcpy( (char*)malloc( sizeof(char)*(strlen(command1)+1) )  , command1 ));
			tokens2=get_tokens( strcpy( (char*)malloc( sizeof(char)*(strlen(command2)+1) )  , command2 ));
			//DisplayTokens(tokens1);
			//DisplayTokens(tokens2);
			

			pipe(pipefd);
			pid=fork();
			
			 if (pid == 0){
			      // child gets here and handles "grep Villanova"

			      // replace standard input with input part of pipe

			      dup2(pipefd[0], 0);

			      // close unused hald of pipe

			      close(pipefd[1]);

      				// execute grep

      				execvp(tokens2[0], tokens2);
 			}
		}
		
		
		
		LocVar *lv=findLocVar( tokens[0] );
		
		if( lv!=NULL){
			tokens=get_tokens( strcpy( (char*)malloc( sizeof(char)*(strlen(str)+1) )  , lv->value ));
		}
		cpid=fork();
		if(cpid==-1){
			perror("fork\n");
			exit(EXIT_FAILURE);
		}else if(cpid!=0){
			if(str[strlen(str)-1]!='$'){
				waitpid(-1, &status, 0); 
			}else{
				printf("deamon id:%d name:%s\n" , cpid , str);
			}
		}else{
			if( is_rdr_r ){
				int fd=creat(output , S_IRWXU );
				if(fd==-1){error("file%s creat failure\n" , output);exit(0);}
				dup2(fd , 1 );
				close(fd);
                        }else if( is_rdr_l ){
                                int fd=open(output , O_RDONLY );
                                if(fd==-1){error("file%s open failure\n" , output);exit(0);}
                                dup2(fd , 0 );
                                close(fd);
                        }else if( is_rdr_rr ){
                                
				 int fd;
                                if( (fd=open(output  ,O_RDWR))==-1){
                                        if( (fd=creat(output , 0666))==-1){error("cannot creat %s",output);}
                                        close(fd);
                                         fd=open(output  ,O_RDWR);
                                        if(fd<0){
                                                printf("Error in lseek\n");
                                                printf("%s", strerror(errno));
                                                return 1;
                                        }

                                }


				 if( lseek (fd, 0L, 2) < 0){
				        printf("Error in lseek\n");
				        printf("%s", strerror(errno));
				        return 1;
				 }
				
				/* error */
				//..........
				
				dup2(fd , 1 );

                                //close(fd);
                        }
			if(ispipe==1){				  // parent gets here and handles "cat scores"

                              // replace standard output with output part of pipe

                              dup2(pipefd[1], 1);

                              // close unused unput half of pipe

                              close(pipefd[0]);

                              // execute cat

                              execvp(tokens1[0], tokens1);
			
			}

			
			execvp(tokens[0], tokens);//argv);
			break ; 
		}
	}
}
