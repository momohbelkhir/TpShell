//shell et couteau suisse:
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <sys/types.h>
#include <unistd.h>
#include <limits.h> 
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>


int redir_cmd(char* tab[],char *in,char* out){ //gestion des redirection > < : elle prend un tab et 2 char* qui indiquent les nom des
    // fichiers d'entrée ou sortis s'ils existent, Null sinon.
    printf("%s %s %s \n",tab[0],in,out); // test d'affichage
    int desc=5;     
    int ec =6;
    dup2(1,desc);   //conserver la sortie standard
    dup2(0,ec);     //conserver l'entrée standard
    
    pid_t pid;                           
    if((pid=fork())==0){
        if(in != NULL && out !=NULL){ // si le in et le out sont null on execute la commande normalement
            close(0); 
            open(in,O_RDONLY);
            close(1);
            open(out,O_WRONLY);
            
            execvp(tab[0],tab);   
            exit(0);
        }else if(out ==NULL && in !=NULL){  
            close(0);
            open(in,O_RDONLY);
            
            execvp(tab[0],tab);   
            exit(0);
        }else if(out !=NULL && in ==NULL){
            close(1);
            open(out,O_WRONLY);
            
            execvp(tab[0],tab);   
            exit(0);
        }
    }else if(pid>0){
        
        
        waitpid(pid,NULL,0);
        dup2(desc,1);  // on recupère la sortie standard
        if(in !=NULL){
            dup2(ec,0);} //on recupere l'entrée standard que si on l'avait fermé précédement
            
    } 
    return 0;
}



int simple_cmd(char *tab[]){  // exécussions de commandes simples:
    
    pid_t pid;
    
    
    
    if(strcmp(tab[0],"cd")==0){ // commande cd:
        char s[300];
        
        chdir(tab[1]); // remplace le répertoire de travail courant du processus appelant par celui indiqué dans tab[1]. 
        getcwd(s,300);
        printf("changement : %s\n",s); 
        
    }else{
        
        if((pid=fork())==0){  // création de processus pour les exécussion de commandes:
            
            execvp(tab[0],tab);
            exit(0);
            
        }else if(pid>0){
            waitpid(pid,NULL,0);
        }
    }
    return 0;
}




int parse_line ( char *s){  //  fonction principale qui prend la chaine de caractère tapé et appelle les autres fonctions: 
    
    int i=0;  
    int j=0;   
    int y=0;
    
    char *tab2[60];  
    char *s2=malloc(60); 
    
    char *env1=malloc(60); //poiteurs pour les varibles d'environnement
    char *env2=malloc(60);
    char *env3=malloc(60);
    
    char* in=NULL;   // pointeur pour les fichiers de redirection
    char* out=NULL;
    
    
    while( s[i] != '\0'&& s[i] != '#'&& s[i] != '\n'){ //s'arreter a la fin ou si on rencontre un # ou un retour a la ligne:
        tab2[j] = malloc(60);
        if(s[i] != ' '){
            s2[y]=s[i];
            y++;
        }else{
            if((strcmp(s2,"exit"))==0 && j==0){ // s'arreter si on recontre "exit"
                return 1;
            }
            
            strcpy(tab2[j],s2); // copier la chaine s2 dans tab2[j].
            free(s2);
            s2=malloc(60);
            j++;
            y=0;      
        }
        i++;
    }  
    if((strcmp(s2,"exit"))==0 && j==0){ // si la chaine contient que exit 
        return 1;
    }
    
    tab2[j] = malloc(60);
    strcpy(tab2[j],s2);      // prendre le dernier élément de la chaine.
    
    tab2[j+1]=NULL;      //mettre la dernière case a null
    
    int k=false;       // k sert pour savoir qu'elle fonction il faudra appeler
    
    for(int m=0;m<=j;m++){
        
        if((strcmp(tab2[m],">"))==0){ //mettre se qui est après le > dans out
            out=malloc(60);
            strcpy(out,tab2[m+1]); 
            k=true;
        }else if((strcmp(tab2[m],"<"))==0 ){ //mettre se qui est après le < dans in
            in=malloc(60);
            strcpy(in,tab2[m+1]); 
            k=true;
        }
        
    }
    //////start manipulation de var d'environement
    
    for(int n=0;n<=j;n++){                    //chercher les var d'environement.        
        if(strpbrk(tab2[n],"=") != NULL){     // séparer ce qui est avant et après le signe =
            char *p=malloc(60);
            int h=0;
            strcpy(p,tab2[n]);
            
            while(p[h] != '='){
                env1[h]=p[h];
                h++;
            }
            h=h+1;
            int f=0;
            while(p[h]!= '\0'){
                env2[f]=p[h];
                
                f++;
                h++;
            }
            setenv(env1,env2,0);
            //printf("env1: %s env2: %s set=%d\n",env1,env2,setenv(env1,env2,0)); 
            
            
        }else if(strpbrk(tab2[n],"$")!= NULL){  // afficher la valeur de la var d'environnement :
            char *p=malloc(60);
            int v=1;
            int y=0;
            strcpy(p,tab2[n]);
            
            while(p[v] != '\0'){ // récuperer ce qui est juste après le $...
                env3[y]=p[v];
                v++;
                y++;
            }      
            if(getenv(env3)!=NULL)
                printf(" %s\n",getenv(env3)); // afficher la valeur de la var d'environnement.
                
        }
    }
    ///fin
    
    
    if(k){                 //  si k est = "true" donc la chaine tapé contient < et/ou > alors on appelle "redir_cmd()".
        char* t[2];
        t[0]=malloc(60);
        strcpy(t[0],tab2[0]);
        t[1]=NULL;
        redir_cmd(t,in,out);
    }else if( (strpbrk(s,"|")) != NULL){
        // parse_pipe(tab2);
    }else {
        simple_cmd(tab2);            // si la chaine est simple ou contient des commandes simple.
    }
    return 0;
}


int main(int argc,char *argv[]){
    
    if(argc>1){                    // manipulation de script de fichier
        
        FILE* pf = fopen(argv[1],"r");
        if(pf!=NULL){
            char *s=malloc(60); 
            while(fgets (s,60, pf) != NULL && (feof(pf)==0)){      // lire dans argv[1] tant que ce n'est pas la fin
                
                if((strpbrk(s,"exit")) != NULL){            // s'arreter a la première ligne qui contient exit
                    return 0;
                }
                parse_line(s);
                free(s);  
                s=malloc(60); 
            }
            
            fclose(pf);
            return 0;
        }else{printf("Le fichier %s n'existes pas!\n",argv[1]);
            
            
        }
        
    }else if(argc==1){
        int a=0;
        while(a==0){      // envoyer la chaine tapé à parse_line();
            printf("$ ");
            char *s=malloc(60);    
            fgets (s,60, stdin);
            a=parse_line(s);
            free(s);
            
        }
    }
    return 0;
}
