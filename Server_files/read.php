<?php
ini_set('display_errors', 1);
ini_set('display_startup_errors', 1);
error_reporting(E_ALL);


// Load DB credentials securely
$config = include('/home/adamnet/config_keys/config.php');

// Create database connection
$conn = new mysqli(
    $config['host'],
    $config['user'],
    $config['password'],
    $config['database']
);

// Check connection
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
}

// Prepare query — modify table name if needed
$sql = "SELECT id, temp, hum, light, source, timestamp FROM logs ORDER BY timestamp DESC LIMIT 50";
$result = $conn->query($sql);

// Display results
if ($result->num_rows > 0) {
    echo "<h2>Latest Logs</h2>";
    echo "<table border='1' cellpadding='6'>";
    echo "<tr><th>ID</th><th>Temp (°C)</th><th>Humidity (%)</th><th>Light</th><th>Source</th><th>Time</th></tr>";

    while ($row = $result->fetch_assoc()) {
        echo "<tr>";
        echo "<td>" . htmlspecialchars($row["id"]) . "</td>";
        echo "<td>" . htmlspecialchars($row["temp"]) . "</td>";
        echo "<td>" . htmlspecialchars($row["hum"]) . "</td>";
        echo "<td>" . htmlspecialchars($row["light"]) . "</td>";
        echo "<td>" . htmlspecialchars($row["source"]) . "</td>";
        echo "<td>" . htmlspecialchars($row["timestamp"]) . "</td>";
        echo "</tr>";
    }

    echo "</table>";
} else {
    echo "No data found.";
}

$conn->close();
?>