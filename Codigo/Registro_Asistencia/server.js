require("dotenv").config(); // Permite usar variables de entorno desde archivo .env
const express = require("express"); // Framework para crear servidor web
const { MongoClient } = require("mongodb"); // Cliente oficial de MongoDB
const dns = require("dns"); // Módulo para configurar DNS

const app = express();

// Permite recibir datos en formato JSON en las peticiones
app.use(express.json());

// ========================
// CONFIGURACIÓN DNS
// ========================
// Se usan DNS de Google (útil cuando MongoDB Atlas da problemas de conexión)
dns.setServers(["8.8.8.8", "8.8.4.4"]);


// ========================
// CONEXIÓN A MONGODB
// ========================

// URI de conexión (se obtiene desde .env)
const uri = process.env.MONGO_URI;

// Crear cliente de MongoDB
const client = new MongoClient(uri);

// Variable global para la base de datos
let db;


// ========================
// USUARIOS RFID (SIMULACIÓN)
// ========================
// Relaciona UID de tarjeta con nombre de usuario
const usuarios = {
  "ba5c86": "Fernando",             //Autor del proyecto
  "1288f4": "Christopher Larkin",   //Compositor de la banda sonora de "Hollow Knight: Silksong"
  "be2dc86": "Walter White",        //Personaje de "Breaking Bad"
  "15adb96": "Victor Borboa",       //Vocalista (Canta una de mis caciones favoritas del juego ↙️: "Une vie à peindre")
  "e715c86": "Verso Dessendre"      //Personaje del juego "Clair Obscur: Expedition 33"
};


// ========================
// FUNCIÓN DE CONEXIÓN A DB
// ========================
async function conectarDB() {
  try {
    // Conectar al servidor de MongoDB
    await client.connect();

    // Seleccionar base de datos
    db = client.db("SensoresDB");

    console.log("Conectado a MongoDB");

  } catch (error) {
    console.error("Error al conectar:", error);
  }
}

// Ejecutar conexión
conectarDB();

// ========================
// ENDPOINT RFID
// ========================
// Recibe UID desde el ESP32
app.post("/api/rfid", async (req, res) => {
  try {

    // Validar que la DB esté conectada
    if (!db) {
      return res.status(500).json({ mensaje: "DB no conectada" });
    }

    // Obtener UID del body
    const { uid } = req.body;

    // Validar que venga el UID
    if (!uid) {
      return res.status(400).json({ mensaje: "UID requerido" });
    }

    // Buscar nombre del usuario (si no existe, "Desconocido")
    const nombre = usuarios[uid] || "Desconocido";

    // ========================
    // LÓGICA DE ACCESO (SIMULADA)
    // ========================
    // Genera acceso aleatorio (50% permitido, 50% denegado)
    const acceso = Math.random() < 0.5;

    // Crear objeto de registro
    const registro = {
      uid, // UID de la tarjeta
      nombre, // Nombre asociado
      acceso, // true = permitido, false = denegado
      fecha: new Date(),  // Fecha y hora actual
      ubicacion: {        // No encontre un manera de obtener la ubicación del ESP32, así que puse una ubicación fija (la universidad)
        lat: 21.617546,   // Latitud
        lng: -102.998003  // Longitud
      }
    };

    // Insertar registro en colección "Accesos"
    const resultado = await db.collection("Accesos").insertOne(registro);

    // Responder al ESP32
    res.json({
      acceso,
      nombre
    });

  } catch (error) {
    console.error(error);

    // Error interno del servidor
    res.status(500).json({ mensaje: "Error servidor" });
  }
});


// ========================
// ENDPOINT PARA VER DATOS
// ========================
app.get("/prueba", async (req, res) => {
  try {

    if (!db) {
      return res.status(500).json({ mensaje: "DB no conectada" });
    }

    const datos = await db
      .collection("Accesos")
      .find()
      .sort({ fecha: -1 }) // más recientes primero
      .toArray();

    res.json(datos);

  } catch (error) {
    console.error(error);
    res.status(500).json({ mensaje: "Error al obtener datos" });
  }
});

// ========================
// ENDPOINT DE PRUEBA
// ========================
// Sirve para verificar que el servidor funciona
app.get("/", (req, res) => {
  res.send("Servidor funcionando 🚀");
});


// ========================
// INICIAR SERVIDOR
// ========================
const PORT = process.env.PORT || 3000;

app.listen(PORT, () => {
  console.log(`Servidor corriendo en puerto ${PORT}`);
});