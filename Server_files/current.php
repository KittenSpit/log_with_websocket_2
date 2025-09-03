<?php
// Load DB credentials securely
$config = include('/home/adamnet/config_keys/config.php');

// Create DSN string
$dsn = "mysql:host={$config['host']};dbname={$config['database']};charset={$config['charset']}";

// PDO options
$options = [
    PDO::ATTR_ERRMODE            => PDO::ERRMODE_EXCEPTION,
    PDO::ATTR_DEFAULT_FETCH_MODE => PDO::FETCH_ASSOC,
];

try {
    // Connect to the database
    $pdo = new PDO($dsn, $config['user'], $config['password'], $options);

    // Fetch the most recent record
    $stmt = $pdo->query("SELECT * FROM logs ORDER BY timestamp DESC LIMIT 1");
    $latest = $stmt->fetch();

} catch (PDOException $e) {
    die("Database connection failed: " . $e->getMessage());
}
?>

<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Latest Record</title>
    <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/css/bootstrap.min.css" rel="stylesheet">
</head>
<body class="bg-light">
<div class="container mt-5">
    <h1 class="mb-4">Most Recent Record</h1>

    <?php if ($latest): ?>
        <div class="card shadow">
            <div class="card-body">
                <?php foreach ($latest as $key => $value): ?>
                    <p>
                        <strong><?= htmlspecialchars(ucwords(str_replace('_', ' ', $key))) ?>:</strong>
                        <?= htmlspecialchars($value) ?>
                    </p>
                <?php endforeach; ?>
            </div>
        </div>
    <?php else: ?>
        <div class="alert alert-warning">No data found in the database.</div>
    <?php endif; ?>
</div>
</body>
</html>
