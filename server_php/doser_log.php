<?php
// server_php/doser_log.php
// Simple endpoint to accept JSON log events and write to MySQL via PDO.
// SQL example:
// CREATE TABLE doser_log (
//   id INT AUTO_INCREMENT PRIMARY KEY,
//   datetime DATETIME NOT NULL,
//   pump INT NOT NULL,
//   requested_ml DECIMAL(10,3),
//   current_ml_per_sec DECIMAL(10,3),
//   current_sec DECIMAL(10,3),
//   event VARCHAR(16) NOT NULL
// );
header('Content-Type: application/json');

$DB_HOST = "localhost";
$DB_NAME = "aquarium";
$DB_USER = "aquarium_user";
$DB_PASS = "REPLACE_WITH_STRONG_PASSWORD";

try {
  $pdo = new PDO("mysql:host=$DB_HOST;dbname=$DB_NAME;charset=utf8mb4", $DB_USER, $DB_PASS, [
    PDO::ATTR_ERRMODE => PDO::ERRMODE_EXCEPTION,
    PDO::ATTR_DEFAULT_FETCH_MODE => PDO::FETCH_ASSOC
  ]);
} catch (Exception $e) {
  http_response_code(500);
  echo json_encode(["ok"=>false, "error"=>"DB connect failed"]);
  exit;
}

$raw = file_get_contents('php://input');
if(!$raw){
  http_response_code(400);
  echo json_encode(["ok"=>false, "error"=>"no body"]);
  exit;
}

$j = json_decode($raw, true);
if(!$j){
  http_response_code(400);
  echo json_encode(["ok"=>false, "error"=>"bad json"]);
  exit;
}

$datetime = $j["datetime"] ?? null;
$pump = isset($j["pump"]) ? intval($j["pump"]) : null;
$requested_ml = isset($j["requested_ml"]) ? floatval($j["requested_ml"]) : null;
$current_ml_per_sec = isset($j["current_ml_per_sec"]) ? floatval($j["current_ml_per_sec"]) : null;
$current_sec = isset($j["current_sec"]) ? floatval($j["current_sec"]) : null;
$event = $j["event"] ?? null;

if(!$datetime || $pump===null || !$event){
  http_response_code(400);
  echo json_encode(["ok"=>false, "error"=>"missing required fields"]);
  exit;
}

try{
  $stmt = $pdo->prepare("INSERT INTO doser_log(datetime,pump,requested_ml,current_ml_per_sec,current_sec,event) VALUES(?,?,?,?,?,?)");
  $stmt->execute([$datetime,$pump,$requested_ml,$current_ml_per_sec,$current_sec,$event]);
  echo json_encode(["ok"=>true]);
} catch (Exception $e){
  http_response_code(500);
  echo json_encode(["ok"=>false, "error"=>"insert failed"]);
}
