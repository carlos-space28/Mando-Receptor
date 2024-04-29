//LECTURA SEÑAL PPM DEL MANDO RC RECIBIDA EN EL RECEPTOR

#define num_canales 8  //Este número de canales variará en función de cuantos recibamos nosotros con nuestra señal PPM. 
uint64_t t_flanco[num_canales * 2 + 2];  //Tendremos un número de flancos que vendrá determinado por el número de canales*2, debido a que cada canal tiene asociado 2 flancos y a estos
//habrá que sumarles el flanco 0 y 17 de la señal PPM que no participan activamente, pero se han de poner como se verá más adelante. 
uint16_t t_canal[num_canales];  //Es el array con la diferencia de los tiempos entre flancos consecutivos, es decir, es la duración del pulso para cada canal o movimiento
uint8_t contador = 1; //Variable que usaremos para crear el array de tiempos en el que se dan los flancos. Inicialmente se inicializa a 1
uint8_t j =1;  //Variable usada para representar el número de Ráfagas que vemos. Cada ráfaga se corresponde a un movimiento en nuestras palancas del mando RC

void setup()
{
  Serial.begin(115200);  //Me permite ver en pantalla los valores de la duración de los pulsos, que es nuestra variable t_canal haciendo referencia al tiempo que dura el pulso para ese canal
  pinMode(2, INPUT);  //Pin en el que leo su estado y veo si cambia para conocer si existen interrupciones.
  attachInterrupt(digitalPinToInterrupt(2), ppm_Interrupt, CHANGE);  //Asocia una interrupción a la lectura de un PIN, en este caso el 2, y cada vez que tengamos un cambio de estado de este pin ejecuta la función ppm_Interrupt
  t_flanco[0] = micros();  //Inicializo el tiempo del primer flanco a micros que tendrá valor practicamente nulo
}

void loop() {
  if (contador == 18) {
    Serial.print("Ráfaga");  //Estas líneas solamente se incluyen una de las primeras veces que se corre este código para identificar las ráfagas 
    //y el valor de los tiempos en microsegundos asociado a cada canal. Después de comprobar que se muestran datos y que son consistentes se comentarán estas líneas
    Serial.print("\t");
    Serial.println(j);
    j++;
    
    for (int i = 1; i <= num_canales; i++) {
      t_canal[i] = t_flanco[2 * i] - t_flanco[2 * i - 1];  //Obtengo la duración de los pulsos de cada canal, es decir, el tiempo asociado a cada canal
      Serial.print(t_canal[i]); //Lo muestro por pantalla
      Serial.print("\t");  //Dejo un espacio de tabulación entre valores de los tiempos de los pulsos
    }
    Serial.println(); //Al acabar de escribir los 8 valores de los 8 canales, salto de linea.
  }
}

void ppm_Interrupt() {  //Se ejecuta solamente si existe una interrupción, es decir, un cambio de estado en el PIN 2
  /*Esta primer línea sirve para identificar nuevas ráfagas. Sabemos que la duración máxima de pulso será de 2000 microsegundos. Todo lo que supere ese valor de duración entre 
  el tiempo actual y tiempo de flanco será porque es una nueva ráfaga. Dentro ya de la propia ráfaga esta condición nunca se cumplirá y por lo tanto el contador nunca se volverá a 
  inicializar a cero dentro de la ráfaga. Solamente se inicializará a cero el contador cuando estemos ante una nueva ráfaga. La primera vez que se introduce en esta función tendrá 
  que esperar 2500 microsegundos en el caso de que se le diera un movimiento a los sticks desde el tiempo inicial del programa(0 segundos) y, después, lo que se hará es mirar siempre 
  que el tiempo actual menos el tiempo del último flanco sea mayor que 2000 microsegundos como mínimo. Se ha puesto 2500 microsegundos como se podría haber puesto 2250 por ejemplo. 
  Con que sea mayor a 2000 microsegundos para saber que estamos en una nueva ráfaga, pero cercano a este valor valdría. 
  */
  
  if ((micros() - t_flanco[0] > 2500) && (micros() - t_flanco[17] > 2500)) contador = 0;
  t_flanco[contador] = micros();  
  contador++; //Actualizo el valor de la variable cada vez que se identifica que se tiene una interrupción. Tengo 18 interrupciones por ráfaga
}

/*
La función ppm_Interrupt se ejecuta cuando se produce una interrupción,es decir, cuando en el pin 2 hay un cambio de estado de HIGH a LOW o de LOW a HIGH. Estas interrupciones me permiten
que mi código principal se siga corriendo hasta que se produzca esa interrupción, momento en el cual ejecutamos esta función. Al terminar de ejecutar esta función sale y sigue ejecutando
el loop principal donde lo dejo. Cada una de las veces que hay una interrupción se toma el tiempo en el que se produce esta interrupción con la función micros() que se guarda en 
la variable t_flanco y se actualiza el valor del contador. En la última iterada obtenemos t_flanco[17] y un valor de la variable contador=18. Con este valor de la variable
ya podemos meternos en el loop princpal, loop que nunca se ejecuta si no tenemos contador ==18, es decir, que no se ejecuta hasta que no hayamos recibido toda nuestra ráfaga, donde 
una ráfaga equivale a los tiempos de los 18 flancos. De estos 18 flancos que se generan, el primero y el último carecen de importancia y son el resto, desde el 2 al 17 con los que se 
opera para obtener las duraciones de los pulsos, que nos indican como se han accionado las palancas. Tendremos un número de pulsos en una ráfaga igual al número de canales. Estos valores
de los pulsos nos indican como de accionadas están las palancas, donde sabemos que un valor de 2ms indicará que la palanca se encuentra en su máximo y 1ms que se encuentra en su 
mínimo. De esta manera conseguimos traducir, un movimiento de las palancas en un valor en ms, y este valor en ms según este más cerca o más lejos de 1 o 2ms nos indicará como de
accionados están los sticks del mando de RC.

*/