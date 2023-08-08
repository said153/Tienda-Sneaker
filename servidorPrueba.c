#include <stdbool.h>
#include <curses.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <mysql/mysql.h>
#define MAX_PRODUCTOS 100

typedef struct nodos{
    int idProducto;
    char nombre[50];
    char marca[50];
    int cantidad;
    double precio;
} Producto;

typedef struct nodos1{
    int idCliente;
    int idProducto;
    int idCarrito;
    int cantidad;
    double precio;
    double precioFinal;
    bool resultado;
}   Carrito;

typedef struct nodos2{
    int idCliente;
    char usuario[50];
    char password[100];
    bool resultado;
} Usuario;

typedef struct  nodos3{
    int opcionMenu;
    int cantidadMenu;
    int idProductoMenu;
    bool resultado;
} Menu;
void actualizarExistencia(Usuario* cliente, int idProducto, int cantidad);

// Funciones de semaforos
int Crea_semaforo(key_t llave,int valor_inicial) {
    int semid=semget(llave,1,IPC_CREAT|0644);
    if(semid==-1)
        return -1;

   semctl(semid,0,SETVAL,valor_inicial);
   
   return semid;
}

void down(int semid) {
    struct sembuf op_p[]={0,-1,0};
    semop(semid,op_p,1);
}

void up(int semid) {
    struct sembuf op_v[]={0,+1,0};
    semop(semid,op_v,1);
}

bool verificarUsuario(Usuario* cliente){
    printf("Se realizara verificar usuario: \n");
    MYSQL *conn;                                                            
    conn = mysql_init(NULL);
    char consulta[200];

    if (conn == NULL)
        printf("Error al inicializar la conexión: %s\n", mysql_error(conn));

//********************************************************************************************
//*****************************Consulta del usuario*******************************************
    if (mysql_real_connect(conn, "localhost", "root", "S@id153*", "Operativos", 0, NULL, 0) == NULL)
        printf("Error al conectarse a la base de datos: %s\n", mysql_error(conn));    
    
    sprintf(consulta, "SELECT id_cliente FROM cliente WHERE usuario = '%s' ", cliente->usuario);

    if (mysql_query(conn, consulta))
    {
        printf("Error al ejecutar la consulta %s\n", mysql_error(conn));
        mysql_close(conn);
    }

    MYSQL_RES *resultado = mysql_store_result(conn);
    if(resultado == NULL){
        printf("Error al ejecutar la consulta %s\n", mysql_error(conn));
        mysql_close(conn);
    }
    MYSQL_ROW fila;
    fila = mysql_fetch_row(resultado);
    char consulta1[200];
//********************************************************************************************
//*****************************Consulta de contraseña*****************************************
    sprintf(consulta1, "SELECT id_cliente FROM cliente WHERE contraseña = '%s'", cliente->password);
    if (mysql_query(conn, consulta1))
    {
        printf("Error al ejecutar la consulta %s\n", mysql_error(conn));
        mysql_close(conn);
    }
    MYSQL_RES *resultado1 = mysql_store_result(conn);
    if(resultado1 == NULL){
        printf("Error al ejecutar la consulta %s\n", mysql_error(conn));
        mysql_close(conn);
    }
    MYSQL_ROW fila1;
    fila1 = mysql_fetch_row(resultado1);
    if ((fila != NULL) && (fila1 != NULL)){
        int id = atoi(fila[0]);
        printf("Usuario valido, %d\n", id);
        cliente->idCliente = id;
        printf("%d\n", cliente->idCliente);
        
        mysql_free_result(resultado);  // Liberar el resultado de la consulta
        mysql_free_result(resultado1);  // Liberar el resultado de la consulta
        mysql_close(conn);  // Cerr
        return true;
    }
    else{
        printf("Usuario Invalido\n");
        mysql_free_result(resultado);  // Liberar el resultado de la consulta
        mysql_free_result(resultado1);  // Liberar el resultado de la consulta
        mysql_close(conn);  
         return false;
    }
}

void mostrarCatalogo(Producto* productos) {
    MYSQL* conn;
    MYSQL_RES* resultado;
    MYSQL_ROW fila;
    char consulta[200];
    conn = mysql_init(NULL);
    if (conn == NULL)
        fprintf(stderr, "Error al inicializar la conexión: %s\n", mysql_error(conn));
        
    if (mysql_real_connect(conn, "localhost", "root", "S@id153*", "Operativos", 0, NULL, 0) == NULL) 
        fprintf(stderr, "Error al conectar con la base de datos: %s\n", mysql_error(conn));
    
    sprintf(consulta, "SELECT * FROM producto");
    if (mysql_query(conn, consulta))
        printf("Error al ejecutar la consulta: %s\n", mysql_error(conn));

    resultado = mysql_store_result(conn);
    if (resultado == NULL)
        printf("Error al obtener el resultado de la consulta: %s\n", mysql_error(conn));
    
    int num_registros = mysql_num_rows(resultado);
    
    if(num_registros == 0)
        printf("No hay productos disponibles\n");
    
    else{
        int i = 0;
        while ((fila = mysql_fetch_row(resultado))) {
        productos[i].idProducto = atoi(fila[0]);
        strncpy(productos[i].nombre, fila[1], sizeof(productos[i].nombre) - 1);
        strncpy(productos[i].marca, fila[2], sizeof(productos[i].marca) - 1);
        productos[i].cantidad = atoi(fila[3]);
        productos[i].precio = atof(fila[4]);
        i++;
        }
    }
    
    mysql_free_result(resultado);
    mysql_close(conn);
}

int verificarExistenciaProducto(MYSQL *conn, int id) {
    // Verificar si el producto existe en la base de datos
    char consultaExistencia[100];
    sprintf(consultaExistencia, "SELECT * FROM producto WHERE id_producto = %d", id);

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

int verificarCantidad(int id_Producto, int cantidad) {
    MYSQL *con = mysql_init(NULL);
    // Conectar a la base de datos
    if (con == NULL) {
        fprintf(stderr, "Error al inicializar la conexión a la base de datos: %s\n", mysql_error(con));
        return -1;
    }
    if (mysql_real_connect(con, "localhost", "root", "S@id153*", "Operativos", 0, NULL, 0) == NULL) {
        fprintf(stderr, "Error al conectar a la base de datos: %s\n", mysql_error(con));
        mysql_close(con);
        return -1;
    }
    // Consulta SQL para obtener la cantidad del producto en la base de datos
    char consulta[100];
    sprintf(consulta, "SELECT cantidad FROM producto WHERE id_producto = %d", id_Producto);
    if (mysql_query(con, consulta)) {
        fprintf(stderr, "Error al ejecutar la consulta: %s\n", mysql_error(con));
        mysql_close(con);
        return -1;
    }
    MYSQL_RES *resultado = mysql_use_result(con);
    if (resultado == NULL) {
        fprintf(stderr, "Error al obtener el resultado de la consulta: %s\n", mysql_error(con));
        mysql_close(con);
        return -1;
    }
    int cantidadBaseDatos = -1;
    MYSQL_ROW fila = mysql_fetch_row(resultado);
    if (fila != NULL) {
        cantidadBaseDatos = atoi(fila[0]);
    }
    mysql_free_result(resultado);
    mysql_close(con);

    // Verificar si la cantidad en la base de datos es mayor que la cantidad pasada como parámetro
    if (cantidadBaseDatos > cantidad) {
        return 0; // La cantidad en la base de datos es mayor
    } else {
        return 1; // La cantidad en la base de datos es menor o igual
    }
}

int verificarExistencia(int id_producto){
    MYSQL *conexion;
    MYSQL_RES *resultado;
    MYSQL_ROW fila;
    int encontrado = 0;
    conexion = mysql_init(NULL);
    if(mysql_real_connect(conexion, "localhost", "root", "S@id153*", "Operativos", 0, NULL, 0) == NULL){
        fprintf(stderr,"Error al conectar a la base de datos");
    }
    char consulta[100];
    snprintf(consulta, sizeof(consulta), "SELECT * FROM producto WHERE id_producto = %d", id_producto);
    if(mysql_query(conexion,consulta)){
        fprintf(stderr,"Error al ejecutar la consulta");
        mysql_close(conexion);
    }
    resultado = mysql_use_result(conexion);
    if ((fila = mysql_fetch_row(resultado))) {
        printf("Producto encontrado\n");
        encontrado = 1;
    } else {
        printf("Producto no encontrado\n");
    }
    // Liberar los recursos
    mysql_free_result(resultado);
    mysql_close(conexion);
    return encontrado;    
}

void agregarCarrito(Usuario *cliente, Menu *miMenu,Carrito *miCarrito){
    int existencia;
    printf("Agregaras al carrito del cliente con id: %d\n", cliente->idCliente);
    printf("El producto con id: %d\n",miMenu->idProductoMenu);
    printf("Con la cantidad de: %d\n",miMenu->cantidadMenu);
    if(verificarExistencia(miMenu->idProductoMenu) == 1){
        if(verificarCantidad(miMenu->idProductoMenu,miMenu->cantidadMenu)==0){
            printf("Digitas te una cantidad correcta");
            actualizarExistencia(cliente, miMenu->idProductoMenu, miMenu->cantidadMenu);
        }
        else{
            printf("Cantidad errronea");
        }
    }
    else{
        printf("Digita un producto valido");
    }
}

void actualizarExistencia(Usuario* cliente, int idProducto, int cantidad) {
    MYSQL* conexion = mysql_init(NULL);
    MYSQL_RES *result;
    MYSQL_ROW row;

    if (conexion == NULL) {
        fprintf(stderr, "Error al inicializar la conexión a la base de datos\n");
        return;
    }

    if (mysql_real_connect(conexion, "localhost", "root", "S@id153*", "Operativos", 0, NULL, 0) == NULL) {
        fprintf(stderr, "Error al conectar a la base de datos: %s\n", mysql_error(conexion));
        mysql_close(conexion);
        return;
    }

    // Construir la consulta SQL para actualizar la tabla "carrito"
    char queryCarrito[200];
    sprintf(queryCarrito, "SELECT COUNT(*) FROM carrito WHERE id_producto = %d AND id_cliente = %d", idProducto, cliente->idCliente);   

    // Ejecutar la consulta para la tabla "carrito"
    if (mysql_query(conexion, queryCarrito) != 0) {
        fprintf(stderr, "Error al ejecutar la consulta: %s\n", mysql_error(conexion));
        mysql_close(conexion);
        return;
    }

    // Obtener el resultado de la consulta para la tabla "carrito"
    result = mysql_use_result(conexion);
    if (result == NULL) {
        fprintf(stderr, "Error al obtener el resultado de la consulta\n");
        mysql_close(conexion);
        return;
    }

    // Leer la fila del resultado
    row = mysql_fetch_row(result);
    int count = atoi(row[0]);

    // Liberar el resultado
    mysql_free_result(result);

    // Imprimir el resultado
    if (count > 0) {
        // Actualizar la tabla "carrito"
        sprintf(queryCarrito, "UPDATE carrito SET cantidad = cantidad + %d WHERE id_producto = %d AND id_cliente = %d", cantidad, idProducto, cliente->idCliente);

        if (mysql_query(conexion, queryCarrito) != 0) {
            fprintf(stderr, "Error al ejecutar la consulta: %s\n", mysql_error(conexion));
            mysql_close(conexion);
            return;
        }
    } else {
        char consultaCarrito[200];
        snprintf(consultaCarrito, sizeof(consultaCarrito), "INSERT INTO carrito (id_cliente, id_producto, cantidad) VALUES (%d, %d, %d) ON DUPLICATE KEY UPDATE cantidad = %d", cliente->idCliente, idProducto, cantidad, cantidad);

        if (mysql_query(conexion, consultaCarrito)) {
            fprintf(stderr, "Error al ejecutar la consulta: %s\n", mysql_error(conexion));
            mysql_close(conexion);
            return;
        }
        printf("Actualización de carrito realizada correctamente.\n");
    }

    // Construir la consulta SQL para actualizar la tabla "producto"
    char queryProducto[200];
    sprintf(queryProducto, "UPDATE producto SET cantidad = cantidad - %d WHERE id_producto = %d", cantidad, idProducto);

    // Ejecutar la consulta para la tabla "producto"
    if (mysql_query(conexion, queryProducto) != 0) {
        fprintf(stderr, "Error al ejecutar la consulta: %s\n", mysql_error(conexion));
        mysql_close(conexion);
        return;
    }

    printf("Actualización de producto realizada correctamente.\n");

    mysql_close(conexion);
} 

void visualizarCarrito(Carrito* miCarrito){
    // Conexión a la base de datos
    MYSQL *conn = mysql_init(NULL);
    if (conn == NULL) {
        fprintf(stderr, "Error al inicializar la conexión a la base de datos: %s\n", mysql_error(conn));
        exit(1);
    }

    if (mysql_real_connect(conn, "localhost", "root", "S@id153*", "Operativos", 0, NULL, 0) == NULL) {
        fprintf(stderr, "Error al conectar a la base de datos: %s\n", mysql_error(conn));
        exit(1);
    }

    // Realizar la consulta a la tabla producto
    if (mysql_query(conn, "SELECT id_carrito,cantidad, precio, total FROM carrito")) {
        fprintf(stderr, "Error al realizar la consulta: %s\n", mysql_error(conn));
        exit(1);
    }

    MYSQL_RES *res = mysql_use_result(conn);
    MYSQL_ROW row;
    size_t num_carrito = 0;

    // Leer todos los registros y guardarlos en la memoria compartida
    while ((row = mysql_fetch_row(res)) != NULL && num_carrito < MAX_PRODUCTOS) {
        miCarrito[num_carrito].idCarrito = atoi(row[0]);
        //strncpy(miCarrito[num_carrito].|, row[1], 50);
        //strncpy(miCarrito[num_carrito].marca, row[2], 50);
        miCarrito[num_carrito].cantidad = atoi(row[3]);
        miCarrito[num_carrito].precio = atof(row[4]);
        num_carrito++;
    }

    mysql_free_result(res);
    mysql_close(conn);

    printf("Se han guardado %lu productos en la memoria compartida.\n", num_carrito);


}

int main() {
    int pasarAlCase=0;
    bool resultado;
    key_t key = 12;
    key_t key2 = 34;
    key_t key3 = 542;
    key_t key4 = 145;
    key_t key_data = 56;  
    key_t key_confirm= 78;
    key_t key_verificacion = 99;
    int opcion,cantidad,id;
//*************************************************************************************************
//*********************************CREACION SEMAFOROS**********************************************

    int sem_data, sem_confirm;
    int sem_verificacion = Crea_semaforo(key_verificacion, 1);    
    sem_data=Crea_semaforo(key_data, 0);
    sem_confirm=Crea_semaforo(key_confirm, 1);
    if(sem_data==-1 || sem_confirm==-1 || sem_verificacion== -1){
        perror("Error al obtener los semaforos");
        return 1;
    }

//*************************************************************************************************
//*****************************MEMORIA COMPARTIDA**************************************************
    Menu *miMenu = NULL;
    int idMemoria1 = shmget(key,sizeof(Menu),IPC_CREAT | 0666);
    if(idMemoria1 == -1){
        perror("\n\tError al obtener la memoria compartida de Menu");
        return 1;
    }

    int idMemoria2 = shmget(key2,MAX_PRODUCTOS*sizeof(Producto), IPC_CREAT | 0777);
    if(idMemoria2 ==-1){
        perror("\n\tError al obtener la memoria compartida de Producto");
        return 1;
    }
    
    int idMemoria3 = shmget(key3, sizeof(Usuario), IPC_CREAT | 0777);
    if(idMemoria3 ==-1){
        perror("\n\tError al obtener la memoria compartida de Usuario");
        return 1;
    }
    int idMemoria4 = shmget(key4,sizeof(Carrito)*10, IPC_CREAT| 0777);
    if(idMemoria4 == -1){
        perror("\n\tError al obtener la memoria compartida de Carrito");
        return 1;
    }
    Usuario *cliente = (Usuario *)shmat(idMemoria3, NULL, 0);
    Producto *productos = (Producto*)shmat(idMemoria2,NULL,0);
    Carrito *miCarrito = (Carrito*)shmat(idMemoria4,NULL,0);
    
    //verificacion= (bool*) shmat(shmid2,0,0);

    while (1) {
        miMenu = (Menu*)shmat(idMemoria1,NULL,0);
        opcion = miMenu->opcionMenu;
        switch (opcion){
            case 1:
            //Codigo para la verificacion del usuario
            printf("\nSe realizara la funcion 1");
            resultado = verificarUsuario(cliente);
            miMenu->resultado = resultado;
            break;
        case 2: 
            //Codigo para mostrar el carrito
            printf("\nSe realizara la funcion 2");
            //visualizar(productos);
            break;
        case 3:
            //Codigo para agregar al carrito
            printf("\nSe realizara la funcion 3");
            //visualizar(productos);
            opcion = 4;
            agregarCarrito(cliente,miMenu,miCarrito);
            pasarAlCase = 1;
            break;
        case 4:
            printf("\nSe realizara ka funcion 4");
            visualizarCarrito(miCarrito);
            break;

        case 5:
            printf("recargando el servidor");
            break;
        default:
            break;
        }
        if(pasarAlCase){
            miMenu->opcionMenu = 5;
            pasarAlCase = 0;
        }
       shmdt(miMenu);
        //shmdt(cliente);        
    }
    shmdt(cliente);
    shmdt(productos);
    shmdt(miCarrito);
    return 0;
}