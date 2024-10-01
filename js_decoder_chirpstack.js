// Decode uplink function for ChirpStack v4
//
// Input is an object with the following fields:
// - bytes = Byte array containing the uplink payload, e.g. [255, 230, 255, 0]
// - fPort = Uplink fPort.
// - recvTime = Uplink message timestamp as Date object.
// - variables = Object containing the configured device variables.
//
// Output must be an object with the following fields:
// - data = Object representing the decoded payload.

function decodeUplink(input) {
    // Asegurarse de que los bytes decodificados tienen la longitud correcta
    if (input.bytes.length !== 4) { // Deben ser 4 bytes: 2 para temp y 2 para humedad
        return {
            errors: ['Invalid payload length']
        };
    }

    // Convertir los primeros dos bytes a un valor de temperatura
    var tempHex = (input.bytes[0] << 8) | input.bytes[1];
    var humHex = (input.bytes[2] << 8) | input.bytes[3];

    // Escalar los valores (dividir por 100) para obtener el valor real
    var temperature = tempHex / 100;
    var humidity = humHex / 100;

    // Devolver los datos decodificados
    return {
        data: {
            temperature: temperature,
            humidity: humidity
        }
    };
}
