/**
 * WiFi Settings.
 */
const char* ssid = "<YOUR WIFI SSID>";
const char* password = "<YOUR WIFI PASSWORD>";

/**
 * API Host url.
 */
const char* ApiHost = "https://api.openai.com/v1/completions";

/**
 * API key token-
 */
const char* chatgpt_token = "<OPENAI API TOKEN>";

/**
 * The Large Language Model that will be used.
 */
const char* chatgpt_model = "gpt-4-turbo";

/**
 * The Text-To-Speech language code.
 */
const char* tts_lang = "en";

/**
 * For TTS we will try to trim down a sentence to the given length if possible.
 * Best results will be obtained with a vaue of 0 (No trim); however, some TTS
 * serivices will cutthe output abruptlly and this value shall be set.
 */
const int max_sentence_length = 80;
