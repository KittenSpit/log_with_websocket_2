<?php
$config = include('/home/adamnet/config_keys/config.php');

$dsn = "mysql:host={$config['host']};dbname={$config['database']};charset={$config['charset']}";

$options = [
    PDO::ATTR_ERRMODE            => PDO::ERRMODE_EXCEPTION,
    PDO::ATTR_DEFAULT_FETCH_MODE => PDO::FETCH_ASSOC,
];

try {
    $pdo = new PDO($dsn, $config['user'], $config['password'], $options);

    $stmt = $pdo->query("SELECT * FROM logs ORDER BY timestamp DESC LIMIT 1");
    $latest = $stmt->fetch();
    $latestId = $latest['id'] ?? null; // assuming you have an `id` column

} catch (PDOException $e) {
    die("Database connection failed: " . $e->getMessage());
}
?>

<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Most Recent Record</title>
    <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/css/bootstrap.min.css" rel="stylesheet">
    <style>
        body { background-color: #f8f9fa; }
        .record-container { max-width: 700px; margin: auto; }
        .table td { vertical-align: middle; }
    </style>
</head>
<body>
<div class="container py-5">
    <div class="record-container">
        <h2 class="mb-4 text-center">📋 Most Recent Record</h2>

        <?php if ($latest): ?>
            <div class="card shadow-sm">
                <div class="card-body">
                    <table class="table table-bordered table-striped">
                        <tbody>
                        <?php foreach ($latest as $key => $value): ?>
                            <tr>
                                <th style="width: 35%;"><?= htmlspecialchars(ucwords(str_replace('_', ' ', $key))) ?></th>
                                <td><?= nl2br(htmlspecialchars($value)) ?></td>
                            </tr>
                        <?php endforeach; ?>
                        </tbody>
                    </table>
                </div>
            </div>
        <?php else: ?>
            <div class="alert alert-warning text-center">⚠️ No data found in the database.</div>
        <?php endif; ?>
    </div>
</div>

<script>
    const currentLatestId = <?= json_encode($latestId) ?>;

    setInterval(() => {
        fetch('check_latest.php')
            .then(response => response.json())
            .then(data => {
                if (data.latest_id && data.latest_id !== currentLatestId) {
                    location.reload(); // New record detected, reload the page
                }
            });
    }, 2000000); // check every 20 seconds
</script>
</body>
</html>
