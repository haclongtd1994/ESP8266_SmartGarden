<?PHP
$VS = "1_0_1"; //mã phiên bản
    
header('Content-type: text/plain; charset=utf8', true);
function check_header($name, $value = false) {
    if(!isset($_SERVER[$name])) {
        return false;
    }
    if($value && $_SERVER[$name] != $value) {
        return false;
    }
    return true;
}
function stream_music($music_url) {
    try {
        // Open the music file
        $file = fopen($music_url, "rb");
        
        // Set the appropriate headers for streaming
        header("Content-Type: audio/mpeg");
        header("Content-Length: " . filesize($music_url));
        header("Content-Disposition: inline; filename=".basename($music_url));
        
        // Stream the music
        fpassthru($file);
        
        // Close the file
        fclose($file);
    } catch (Exception $e) {
        // Log the error
        error_log("Error: " . $e->getMessage());
    }
}
if(!check_header('HTTP_USER_AGENT', 'ESP8266-http-Update')) {
    header($_SERVER["SERVER_PROTOCOL"].' 403 Forbidden', true, 403);
    // header($_SERVER["SERVER_PROTOCOL"].' 200 OK', true, 200);
    // stream_music("file.mp3");
    echo "Chỉ phục vụ esp8266 - 1!";
    exit();
}
if(
    !check_header('HTTP_X_ESP8266_STA_MAC') ||
    !check_header('HTTP_X_ESP8266_AP_MAC') ||
    !check_header('HTTP_X_ESP8266_AP_MAC') ||
    !check_header('HTTP_X_ESP8266_SKETCH_SIZE') ||
    !check_header('HTTP_X_ESP8266_SKETCH_MD5') ||
    !check_header('HTTP_X_ESP8266_CHIP_SIZE') ||
    !check_header('HTTP_X_ESP8266_SDK_VERSION')
) {
    header($_SERVER["SERVER_PROTOCOL"].' 403 Forbidden', true, 403);
    echo "Chỉ phục vụ esp8266 - 2!";
    exit();
}
//kiểm tra phiên  bản
if(check_header('HTTP_X_ESP8266_VERSION', $VS)) {
    header($_SERVER["SERVER_PROTOCOL"].' 403 Forbidden', true, 403); 
    echo "no update - 1";
    exit();
}
function sendFile($path) {
    header($_SERVER["SERVER_PROTOCOL"].' 200 OK', true, 200);
    header('Content-Type: application/octet-stream', true);
    header('Content-Disposition: attachment; filename='.basename($path));
    header('Content-Length: '.filesize($path), true);
    header('x-MD5: '.md5_file($path), true);
    readfile($path);
}
sendFile("firmware.bin");