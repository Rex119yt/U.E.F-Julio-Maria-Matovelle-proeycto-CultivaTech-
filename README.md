# U.E.F Julio María Matovelle proyecto CultivaTech

<p>
Este es un repositorio creado para tener un mejor control sobre las versiones del código del proyecto del Invernadero
</p>

--- 
Este es un proyecto que incorpora la tecnología Iot también llamada "Internet de las cosas" aplicándola en el microcontrolador ESP32, placa seleccionada debido a su natividad para conectarse a redes Wifi.

Se hace uso de un chatbot de Telegram, como un intermediario entre el usuario y los microcontroladores, permitiendo enviar comandos desde cualquier dispositivo con acceso a esta Aplicación

Los archivos que incluye este proyecto son dos

**Código_Invernadero.cpp**

*Es el código principal del proyecto, siendo el encargado de dar lectura a los diferentes sensores que incorpora el invernadero, ademas de activar los distintos dispositivos en el momento que se los requiera, también se encarga de responder a los distintos mensajes enviados al chatbot de Telegram*

**esp32cam-telegram-bot.ino**

*Es el código encargado de inicializar de manera correcta la esp32-cam, para que funcione correctamente, ademas de responder al comando "/foto" enviando una foto del interior del invernadero al chatbot de Telegram*

---
En la sección de commits se encuentra presente un registro de todas las versiones subidas del proyecto disponible para su revisión
