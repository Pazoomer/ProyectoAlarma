/*
 * GestionDatos.ino
 *
 * Este modulo permite el cálculo de los promedios moviles 
 * de una una serie de datos. Los datos a promediar se  
 * insertan en una cola. Una vez que se llena la cola,
 * para insertar un nuevo dato, el dato mas antiguo se 
 * descarta primero.
 */

/**
 * Esta función inserta el dato dado en la cola.
 * tamCola es el tamaño de la cola. Si la cola se
 * llena, para insertar un nuevo dato, el dato mas 
 * antiguo se descarta primero.
 *
 * Regresa el numero de datos en la cola
 */    
int insertaCola(int *pCola, int dato, int tamCola) {
    static int i = 0;
    static bool primera = true;

    // Inserta el dato en la cola
    *(pCola + i) = dato;
    i++;

    // Bandera que indica que es la primera vez que se
    // está llenando la cola
    if(i == tamCola) primera = false;

    // Si la cola se llena, empieza a sobreescribir
    // los datos
    i %= tamCola;

    // Regresa el numero de datos en la cola 
    return primera? i: tamCola;
}

/**
 * Esta función promedia los datos en la cola
 */
float obtenPromedioMovil(int *pCola, int nDatos) {
    if(nDatos <= 0) return 0.0;

    int suma = 0;
    for(int i = 0; i < nDatos; i++)
        suma += *(pCola + i);

    return (float)suma/nDatos;
}      

/**
 * Esta funcion despliega el contenido de la cola
 */
void despliegaCola(int *pCola, int nDatos) {
    Serial.print("[");

    for(int i = 0; i < nDatos - 1; i++) {
        Serial.print(*(pCola + i));
        Serial.print(", ");
    }
    Serial.print(*(pCola + nDatos - 1));
    Serial.println("]");

}

int insertaColaFloat(float *pCola, float dato, int tamCola) {
    static int i = 0;
    static bool primera = true;

    // Inserta el dato en la cola
    *(pCola + i) = dato;
    i++;

    // Bandera que indica que es la primera vez que se
    // está llenando la cola
    if (i == tamCola) primera = false;

    // Si la cola se llena, empieza a sobreescribir
    // los datos
    i %= tamCola;

    // Regresa el número de datos en la cola 
    return primera ? i : tamCola;
}

float obtenPromedioMovilFloat(float *pCola, int nDatos) {
    if (nDatos <= 0) return 0.0;

    float suma = 0;
    for (int i = 0; i < nDatos; i++)
        suma += *(pCola + i);

    return suma / nDatos;
}

void despliegaColaFloat(float *pCola, int nDatos) {
    Serial.print("[");

    for (int i = 0; i < nDatos - 1; i++) {
        Serial.print(*(pCola + i), 2);  // muestra 2 decimales
        Serial.print(", ");
    }
    Serial.print(*(pCola + nDatos - 1), 2);
    Serial.println("]");
}
