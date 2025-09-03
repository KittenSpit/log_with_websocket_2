<?php
ini_set('display_errors', 1);
error_reporting(E_ALL);

// Load config
$config = include('/home/adamnet/config_keys/config.php');

// Connect to DB
$conn = new mysqli(
    $config['host'],
    $config['user'],
    $config['password'],
    $config['database']
);

if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
}

// Get filters from URL (only date filters)
$start = $_GET['start'] ?? null;
$end = $_GET['end'] ?? null;
$export = isset($_GET['export']);

// Base SQL
$sql = "SELECT id, temp, hum, light, source, timestamp FROM logs WHERE 1";
$params = [];
$types = "";

// Add date filters only
if (!empty($start)) {
    $sql .= " AND timestamp >= ?";
    $params[] = $start;
    $types .= "s";
}
if (!empty($end)) {
    $sql .= " AND timestamp <= ?";
    $params[] = $end;
    $types .= "s";
}

$sql .= " ORDER BY timestamp DESC";

// Prepare & bind
$stmt = $conn->prepare($sql);

if ($params) {
    $stmt->bind_param($types, ...$params);
}

$stmt->execute();
$result = $stmt->get_result();

// Handle CSV export
if ($export) {
    header('Content-Type: text/csv');
    header('Content-Disposition: attachment; filename="logs.csv"');
    $output = fopen('php://output', 'w');
    fputcsv($output, ['ID', 'Temp', 'Humidity', 'Light', 'Source', 'Timestamp']);

    while ($row = $result->fetch_assoc()) {
        fputcsv($output, $row);
    }

    fclose($output);
    exit;
}
?>

<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>Logs Viewer</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 30px;
        }
        table {
            width: 100%;
            border-collapse: collapse;
            max-height: 500px;
            overflow-y: auto;
            display: block;
        }
        th, td {
            padding: 8px;
            text-align: center;
            border: 1px solid #ccc;
        }
        form {
            margin-bottom: 20px;
        }
        label {
            margin-right: 10px;
        }
        .scroll-container {
            max-height: 500px;
            overflow-y: auto;
            border: 1px solid #ccc;
        }
    </style>
</head>
<body>
    <h2>Log Viewer</h2>

    <form method="GET" action="">
        <label>Start Date:
            <input type="date" name="start" value="<?= htmlspecialchars($start) ?>">
        </label>
        <label>End Date:
            <input type="date" name="end" value="<?= htmlspecialchars($end) ?>">
        </label>
        <!-- Source filter removed from the form -->
        <button type="submit">Filter</button>
        <button type="submit" name="export" value="1">Export CSV</button>
    </form>

    <div class="scroll-container">
        <table>
            <thead>
                <tr>
                    <th>ID</th>
                    <th>Temp (°C)</th>
                    <th>Humidity (%)</th>
                    <th>Light</th>
                    <th>Source</th> <!-- Source column stays -->
                    <th>Time</th>
                </tr>
            </thead>
            <tbody>
                <?php if ($result->num_rows > 0): ?>
                    <?php while ($row = $result->fetch_assoc()): ?>
                        <tr>
                            <td><?= htmlspecialchars($row['id']) ?></td>
                            <td><?= htmlspecialchars($row['temp']) ?></td>
                            <td><?= htmlspecialchars($row['hum']) ?></td>
                            <td><?= htmlspecialchars($row['light']) ?></td>
                            <td><?= htmlspecialchars($row['source']) ?></td>
                            <td><?= htmlspecialchars($row['timestamp']) ?></td>
                        </tr>
                    <?php endwhile; ?>
                <?php else: ?>
                    <tr><td colspan="6">No data found.</td></tr>
                <?php endif; ?>
            </tbody>
        </table>
    </div>
</body>
</html>

<?php
$conn->close();
?>
