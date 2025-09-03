<?php
$config = include('/home/adamnet/config_keys/config.php');

$dsn = "mysql:host={$config['host']};dbname={$config['database']};charset={$config['charset']}";

$options = [
    PDO::ATTR_ERRMODE            => PDO::ERRMODE_EXCEPTION,
    PDO::ATTR_DEFAULT_FETCH_MODE => PDO::FETCH_ASSOC,
];

try {
    $pdo = new PDO($dsn, $config['user'], $config['password'], $options);

    $stmt = $pdo->query("SELECT id FROM logs ORDER BY timestamp DESC LIMIT 1");
    $row = $stmt->fetch();

    echo json_encode(['latest_id' => $row['id'] ?? null]);

} catch (PDOException $e) {
    echo json_encode(['error' => 'Database error']);
}
