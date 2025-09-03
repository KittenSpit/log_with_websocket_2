<?php

 //Create a connection
$config = include('/home/adamnet/config_keys/config.php');

$conn = new mysqli(
    $config['host'],
    $config['user'],
    $config['password'],
    $config['database']
);

if ($conn->connect_error) {
    error_log("Connection failed: " . $conn->connect_error);
    die("Internal Server Error");
}

// Optional: Check API key if using
//if (!isset($_GET['key']) || $_GET['key'] !== $config['api_key']) {
//    http_response_code(403);
//    die("Forbidden: Invalid API Key");
//}


// Get data from query parameters
$temp = isset($_GET['temp']) ? (float) $_GET['temp'] : 0.0;
$hum = isset($_GET['hum']) ? (float) $_GET['hum'] : 0.0;
$light = isset($_GET['light']) ? $_GET['light'] : 0;
$source = isset($_GET['source']) ? $_GET['source'] : 'Unknown';

// Insert data into MySQL
$stmt = $conn->prepare("INSERT INTO logs (temp, hum, light, source) VALUES (?, ?, ?, ?)");
$stmt->bind_param("ddis", $temp, $hum, $light, $source);
$stmt->execute();

echo "Data logged successfully!";

// Close the connection
$conn->close();
?>
