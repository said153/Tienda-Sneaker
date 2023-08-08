//***************************************Interfaz del servidor********************************
//********************************************************************************************
//********************************************************************************************
#include <mysql/mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <pthread.h>
#include <string.h>
#include <ctype.h>

int Crea_semaforo(key_t,int);
void down(int);
void up(int);
int RegionNoCritica();
int RegionCritica(int, int *);

typedef struct{
    int id;
    char nombre[50];
    char marca[50];
    int cantidad;
} Registro;

typedef struct Nodo{
    int id;
    char nombre[50];
    char marca[50];
    int cantidad;
    double precio;
    double sumaTotal;
    struct Nodo* siguiente;
} Nodo;

void mostrarCatalogo(){
MYSQL *conexion;
MYSQL_RES *resultado;
MYSQL_ROW fila;
int num_registros = 0;
conexion = mysql_init(NULL);
int idMemoriaCompartida;
key_t clave = 5678;
#define PERMISOS 0644

if (conexion == NULL) {
    fprintf(stderr, "Error al inicializar la conexión: %s\n", mysql_error(conexion));
}
else {
    if (mysql_real_connect(conexion, "localhost", "root", "Da18rio", "proyectoOperativos", 0, NULL, 0) == NULL) {
        fprintf(stderr, "Error al conectar con la base de datos: %s\n", mysql_error(conexion));
        mysql_close(conexion);
    }
    else {
        // Ejecutar consulta SELECT para obtener los registros de la tabla "tenis"
        if (mysql_query(conexion, "SELECT * FROM tenis")) {
            fprintf(stderr, "Error al ejecutar la consulta: %s\n", mysql_error(conexion));
            mysql_close(conexion);
        }
        else {
            resultado = mysql_store_result(conexion);
            // Obtener el número de registros
            num_registros = mysql_num_rows(resultado);
            // Crear y obtener la memoria compartida
            idMemoriaCompartida = shmget(clave, sizeof(Registro) * num_registros, 0666 | IPC_CREAT);
            if (idMemoriaCompartida == -1) {
                perror("Error al crear la memoria compartida");
                mysql_free_result(resultado);
                mysql_close(conexion);
            }
            else {
                printf("ID DE LA MEMORIA COMPARTIDA: %d\n", clave);
                // Adjuntar la memoria compartida a un puntero
                Registro* registros = (Registro*)shmat(idMemoriaCompartida, NULL, 0);
                if (registros == (Registro*)-1) {
                    perror("Error al adjuntar la memoria compartida");
                    mysql_free_result(resultado);
                    mysql_close(conexion);
                }
                else {
                    // Copiar los registros de la base de datos al arreglo
                    int i = 0;
                    while ((fila = mysql_fetch_row(resultado)) != NULL) {
                        registros[i].id = atoi(fila[0]);
                        strncpy(registros[i].nombre, fila[1], sizeof(registros[i].nombre) - 1);
                        strncpy(registros[i].marca, fila[2], sizeof(registros[i].marca) - 1);
                        registros[i].cantidad = atoi(fila[3]);
                        i++;
                    }
                    // Liberar la memoria de la consulta y cerrar la conexión
                    mysql_free_result(resultado);
                    mysql_close(conexion);        
                }
            }
        }
    }
}
}

int verificarExistenciaProducto(MYSQL *conn, int id) {
    // Verificar si el producto existe en la base de datos
    char consultaExistencia[100];
    sprintf(consultaExistencia, "SELECT COUNT(*) FROM tenis WHERE id = %d", id);

    if (mysql_query(conn, consultaExistencia)) {
        fprintf(stderr, "Error al ejecutar la consulta: %s\n", mysql_error(conn));
        return 0; // Indicar que ocurrió un error
    }

    MYSQL_RES *resultExistencia = mysql_use_result(conn);
    MYSQL_ROW rowExistencia;
    if ((rowExistencia = mysql_fetch_row(resultExistencia))) {
        int count = atoi(rowExistencia[0]);
        mysql_free_result(resultExistencia);

        return (count > 0); // Devolver 1 si el producto existe, 0 si no existe
    }

    mysql_free_result(resultExistencia);
    return 0; // Indicar que ocurrió un error
}

void agregarCarrito(int id,int valorResta, Nodo** carrito) {
    size_t clave3 = 9876;
    MYSQL* conn = mysql_init(NULL);
    if (conn == NULL) {
        fprintf(stderr, "Error al inicializar la conexión: %s\n", mysql_error(conn));
        return;
    }
    if (mysql_real_connect(conn, "localhost", "root", "Da18rio", "proyectoOperativos", 0, NULL, 0) == NULL) {
        fprintf(stderr, "Error al conectar a la base de datos: %s\n", mysql_error(conn));
        mysql_close(conn);
        return;
    }
    // Verificar la existencia del producto en la base de datos
    if (!verificarExistenciaProducto(conn, id)) {
        fprintf(stderr, "El producto con el ID especificado no existe.\n");
        mysql_close(conn);
        return;
    }
    // Verificar si el producto ya existe en el carrito
    Nodo* nodoActual = *carrito;
    while (nodoActual != NULL) {
        if (nodoActual->id == id) {
            printf("Digite la cantidad: ");
            scanf("%d", &valorResta);
            // Actualizar la cantidad y el precio en la estructura del carrito
            nodoActual->cantidad += valorResta;
            nodoActual->sumaTotal = nodoActual->precio * nodoActual->cantidad;
            // Actualizar la cantidad en la base de datos
            char consultaUpdate[100];
            sprintf(consultaUpdate, "UPDATE tenis SET cantidad = cantidad - %d WHERE id = %d", valorResta, id);
            if (mysql_query(conn, consultaUpdate)) {
                fprintf(stderr, "Error al ejecutar la consulta de actualización: %s\n", mysql_error(conn));
                mysql_close(conn);
                return;
            }
            printf("Se ha actualizado satisfactoriamente la cantidad del producto en el carrito.\n");
            mysql_close(conn);
            return;
        }
        nodoActual = nodoActual->siguiente;
    }
    // Verificar si valorResta es menor o igual a la cantidad actual en la base de datos
    char consultaCantidad[100];
    sprintf(consultaCantidad, "SELECT nombre, precio, cantidad FROM tenis WHERE id = %d", id);
    if (mysql_query(conn, consultaCantidad)) {
        fprintf(stderr, "Error al ejecutar la consulta: %s\n", mysql_error(conn));
        mysql_close(conn);
        return;
    }
    MYSQL_RES* resultCantidad = mysql_use_result(conn);
    MYSQL_ROW rowCantidad;
    if ((rowCantidad = mysql_fetch_row(resultCantidad))) {
        Nodo* nuevoNodo = malloc(sizeof(Nodo));
        nuevoNodo->id = id;
        strcpy(nuevoNodo->nombre, rowCantidad[0]);
        nuevoNodo->precio = atof(rowCantidad[1]);
        nuevoNodo->cantidad = valorResta;
        nuevoNodo->sumaTotal = nuevoNodo->precio * nuevoNodo->cantidad;
        nuevoNodo->siguiente = NULL;
        // Agregar el nuevo nodo al final del carrito
        if (*carrito == NULL) {
            *carrito = nuevoNodo;
        } else {
            nodoActual = *carrito;
            while (nodoActual->siguiente != NULL) {
                nodoActual = nodoActual->siguiente;
            }
            nodoActual->siguiente = nuevoNodo;
        }
        mysql_free_result(resultCantidad); // Liberar los resultados de la consulta
        // Actualizar la cantidad en la base de datos
        char consultaUpdate[100];
        sprintf(consultaUpdate, "UPDATE tenis SET cantidad = cantidad - %d WHERE id = %d", valorResta, id);
        if (mysql_query(conn, consultaUpdate)) {
            fprintf(stderr, "Error al ejecutar la consulta de actualización: %s\n", mysql_error(conn));
            mysql_close(conn);
            return;
        }
        printf("Se ha agregado satisfactoriamente el producto al carrito.\n");
    } else {
        fprintf(stderr, "No se encontró el producto con el ID especificado.\n");
        mysql_free_result(resultCantidad); // Liberar los resultados de la consulta
    }
    mysql_close(conn);
    size_t tamMemoria = sizeof(Nodo);
    if(clave3 ==-1){
        fprintf(stderr,"Error al generar la clave para la memoria compartida");
    }
    else{
        int idMemoria3 = shmget(clave3,tamMemoria,IPC_CREAT | 0666);
        if(idMemoria3 == -1){
            fprintf(stderr,"Error al crear u obtener el identificador");
        }
        else
        {
            Nodo* ptrMemoria = (Nodo*)shmat(idMemoria3,NULL,0);
            if(ptrMemoria == (void*)-1){
                fprintf(stderr,"Error al adjuntar la memoria compartida");
            }
            else
            {
                *ptrMemoria = **carrito;
                shmdt(ptrMemoria);
            }
            
        }
        
    }
    
}

void mostrarEstructuraMemoriaCompartida() {
    size_t clave3 = 9876;
    int idMemoria3 = shmget(clave3, sizeof(Nodo), 0666);
    if (idMemoria3 == -1) {
        fprintf(stderr, "Error al obtener el identificador de la memoria compartida\n");
        return;
    }

    Nodo* ptrMemoria = (Nodo*)shmat(idMemoria3, NULL, 0);
    if (ptrMemoria == (void*)-1) {
        fprintf(stderr, "Error al adjuntar la memoria compartida\n");
        return;
    }

    Nodo* carrito = ptrMemoria;
    if (carrito == NULL) {
        printf("La estructura en la memoria compartida está vacía.\n");
    } else {
        printf("Contenido de la estructura en la memoria compartida:\n");
        int i = 1;
        while (carrito != NULL) {
            printf("Registro %d:\n", i);
            printf("ID: %d\n", carrito->id);
            printf("Nombre: %s\n", carrito->nombre);
            printf("Precio: %.2f\n", carrito->precio);
            printf("Cantidad: %d\n", carrito->cantidad);
            printf("Suma total: %.2f\n", carrito->sumaTotal);
            printf("\n");
            carrito = carrito->siguiente;
            i++;
        }
    }

    shmdt(ptrMemoria);
}

int main(){
	int dato;
	int semaforo_mutex;
   	int shmid1, shmid2;
	int *datos, *resultados;
	key_t llave1 = 123; //clave para la region de memoria compartida
	key_t llave2 = 456; //clave para la region de memoria compartida resultados
	key_t llave3 = 789; //clave para el semaforo



//Creamos el semaforo
	semaforo_mutex = Crea_semaforo(llave3,1);
    
    printf("\n\tConectando el servidor...........\n");
    sleep(5);
    
    while (1){
   		dato = RegionNoCritica(); //Para el identificador 
   		down(semaforo_mutex);
   		RegionCritica(dato, resultados);
   		up(semaforo_mutex); 	
	}
	sleep(20);
    shmdt(datos);
	shmdt(resultados);
	
	shmctl(shmid1, IPC_RMID, 0);
	shmctl(shmid2, IPC_RMID, 0);
	
}

int Crea_semaforo(key_t llave3,int valor)
{
   int semid = semget(llave3 , 1 , IPC_CREAT | PERMISOS);
   if(semid==-1)
      return -1;

   semctl( semid , 0 , SETVAL , valor);
   return semid;
}

void down(int semid)
{
	struct sembuf op_p[] = {0,-1,0};
   	semop(semid,op_p,1);
}

void up(int semid)
{
   	struct sembuf op_v[]={0,+1,0};
   	semop(semid,op_v,1);
}

int RegionNoCritica()
{
   int pid;
    printf("\n\tServidor en linea");
    sleep(5);
    //printf("\nIdentificador del semaforo: %d\n",getpid());
    pid=getpid();
    return pid;
}

int RegionCritica(int dato, int *resultados)
{
	Nodo *miCarrito = NULL;
    key_t clave = 12345;
    //keyt_t clave3 = 9876;
    int idMemoria3 = shmget(clave, sizeof(int) * 3, IPC_CREAT | 0666);
    // Crear la memoria compartida
    int id_memoria = shmget(clave, sizeof(int) * 3, IPC_CREAT | 0666);
    if (id_memoria == -1) {
        perror("Error al crear la memoria compartida");
        exit(EXIT_FAILURE);
    }
    // Adjuntar la memoria compartida
    int *memoria = shmat(id_memoria, NULL, 0);
    if (memoria == (int *) -1) {
        perror("Error al adjuntar la memoria compartida");
        exit(EXIT_FAILURE);
    }
    // Leer los valores de la memoria compartida
    int opcion = memoria[0];
    printf("La opcion desde el inicio es: %d\n",opcion);
        switch (opcion)
        {
            case 1:
                mostrarCatalogo();
            break;
            case 2:
                int id = memoria[1];
                int cantidad = memoria[2];
                agregarCarrito(id,cantidad,&miCarrito);
                mostrarCatalogo();
            break;
            case 3:
            //Se mostrara el carrito 
                mostrarEstructuraMemoriaCompartida();
            break;
            default:
                printf("Error de conexion\n");
            break;
     }
    
    
      shmdt(memoria);
}