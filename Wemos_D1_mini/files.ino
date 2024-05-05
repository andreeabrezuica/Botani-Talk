String readFile(const char* filename) {
  if (!LittleFS.begin()) {
    return String("An Error has occurred while mounting LittleFS");
  }

  File file = LittleFS.open(filename, "r");
  if (!file) {
    return String(FILE_READ_FAILED);
  }
  String data = file.readString();
  file.close();
  return data;
}

bool appendFile(const char* filename, const char* message) {
  if (!LittleFS.begin()) {
    return false;
  }
  File file = LittleFS.open(filename, "a");
  if (!file) {
    return false;
  }
  bool success = file.print(message);
  file.close();
  return success;
}

void updateCSV() {
  Serial.println("Saving CSV file");
  getInfoFromMega();

  String values = String("\n") + plantStatus.moist + "," + plantStatus.light + "," + plantStatus.temp + "," + getUnixTime();
  if (readFile("/sensors.csv").isEmpty()) {
    appendFile("/sensors.csv", "moisture,light,temperature,time\n");
  }
  appendFile("/sensors.csv", values.c_str());
}
