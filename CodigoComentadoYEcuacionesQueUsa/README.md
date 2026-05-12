## **Código Maestro**

En este código el arduino tiene la conexión con el Arduino Esclavo, usando una conexión del tipo RX,TX, con los pines digitales 2 y 3. Además de que esta encargado de realizar todo el análisis, es como el cerebro del sistema. 

Y para comenzar es nesario, definir la latitud de Medellín, la cual se reporta en grados como $6.24°$, por lo que es necesario pasarla a radiones. Luego, la rutina setup, se cersiora de realizar unacomunicación con el serial, el esclavo y el rtc, actualizando el rtc a la hora actual del momento que cuando se carga el código.

Después en la rutina que se repitirá infinitamente (loop): 

* Leer la hora: es necesario extraer primero obtener el día del año, hora, mes, día, minuto y segundo. Para después a través de un conteno de días, sumandolos mes a mes, es decir, enero tiene 31 y febrero tiene 28, sumandolos hacen 59 días. Para después, volver cada día del año un entero, tipo el 15 de marzo, es en enteros $n = 31 + 28 + 15$. Para después reportar la hora, pasando los minutos y segundos a horas, es decir, $14:30:30$ es $14 + 30/60 + 30/3600 = 14.5083$.
* Declinación solar y lunar:
Se presenta la ecuación de Cooper (1969) para calcular la declinación solar $\delta$, la cual se expresa como $\delta =23.45\cdot \sin \left[\frac{360}{365}\cdot (n-81)\cdot \frac{\pi }{180}\right]$ Doonde, $\delta$ (Declinación Solar) es el ángulo resultante expresado en grados que varía entre $+23.45^{\circ }$ (solsticio de verano en el hemisferio norte) y $-23.45^{\circ }$ (solsticio de invierno).Luego $n$ es el día del año o día juliano (1 para el 1 de enero, 365 para el 31 de diciembre). Depués $(n - 81)$, es el desfase temporal, en el cual el día $81$ corresponde al equinoccio de primavera (21 de marzo), momento en el cual la declinación es exactamente 0. Y $\frac{360}{365}$, es el factor que convierte los días transcurridos en grados de arco, asumiendo una órbita circular de 365 días. 
* Ángulo horario: $H = 15  (h - 12)$, donde el $15$ (grados por hora) considera el giro de la Tierra de 360° en 24 horas de la Tierra ($360/24$). Y $h -12$ define el mediodía solar, como el pundo de referencia, es decir, a las $12:00$ $H = 0^{\circ}$.
* Elevación Solar: usando la dey de cosenos para el sistema de coordendas horizontales:  \(\sin (\alpha )=\sin (\phi )\cdot \sin (\delta )+\cos (\phi )\cdot \cos (\delta )\cdot \cos (H)\), donde $\phi$ es la latitud ya mencionada.
* Azimut solar: usando la ecuación \(\cos (A)=\frac{\sin (\delta )-\sin (\alpha )\cdot \sin (\phi )}{\cos (\alpha )\cdot \cos (\phi )}\).

Así, se les da a los servos la posición en elevación y azimut solar, para después leer los resultados de luz capatada por el fotodiodo. E imprimir en el serial, la hora, luz, y la posisicón del servo horizontal y del vertical. Y finalmente enviar al arduino esclavo la posición de los servos. 

## **Código maestro**

El código del Arduino esclavo se encarga de recibir desde el Arduino maestro las posiciones objetivo para los servomotores y moverlos de manera suave y controlada. Primero establece una comunicación serial usando la librería SoftwareSerial, por donde recibe mensajes con el formato "anguloHorizontal,anguloVertical". Cada mensaje enviado por el maestro contiene los ángulos que deben tomar el servo horizontal y el servo vertical.

En el programa, el esclavo lee carácter por carácter la información recibida hasta encontrar un salto de línea (\n), lo que indica que el mensaje llegó completo. Luego separa los datos usando la coma como referencia y convierte cada valor a enteros para almacenarlos como posiciones objetivo (targetH y targetV).

Después, el código implementa un sistema de suavizado o movimiento incremental. En lugar de mover los servos directamente a la posición final, compara la posición actual del servo con la posición deseada y avanza poco a poco usando pequeños incrementos definidos por la variable stepSize. Esto permite que el movimiento sea más fluido, evita vibraciones bruscas y reduce el esfuerzo mecánico sobre los servomotores.

Finalmente, el Arduino actualiza continuamente las posiciones de ambos servos horizontal y vertical, logrando que la estructura del sistema autoguiado siga la posición calculada por el maestro de forma estable y suave.




