#include <ArduinoJson.h>
#include <Braccio++.h>
#include <BraccioIK.h>
#include <Onshape.h>

#define BUTTON_ENTER 6
const PROGMEM char server[] = "cad.onshape.com";

auto gripper = Braccio.get(1);
auto wristRoll = Braccio.get(2);
auto wristPitch = Braccio.get(3);
auto elbow = Braccio.get(4);
auto shoulder = Braccio.get(5);
auto base = Braccio.get(6);

Onshape myclient("YourOnshapeAPIkey",
                 "YourOnshapeAPIpass");
int cpNum = 0;

float motorMin[4] = {0, 84, 55, 38.19};
float motorMax[4] = {360, 225, 256, 251.76};
float angles[6];
float homePos[6] = {230.92, 159.63, 147.26, 152.15, 150.81, 180.42};
float interPos[6] = {230, 160.0, 240.0, 240.0, 100.0, 180.42};

float positions[10][5];
bool monitorArm = true;

IK_Calculator bracciocalc(motorMin, motorMax, homePos);

void setup() {
  if (!Braccio.begin()) {
    if (Serial) Serial.println("Braccio.begin() failed.");
    for (;;) {
    }
  }
  delay(500);

  Serial.println("Braccio Initialized");

  Braccio.speed(SLOW);

  myclient.start("YourWiFiSSID", "YourWiFiPassword");

  Braccio.moveTo(interPos[0], interPos[1], interPos[2], interPos[3],
                 interPos[4], interPos[5]);
  if (monitorArm) {
    setMates(interPos[0], interPos[1], interPos[2], interPos[3], interPos[4],
             interPos[5]);
  } else {
    delay(2000);
  }
  Serial.println(F("Ready"));
}

void loop() {
  if (Braccio.getKey() == BUTTON_ENTER) {
    Serial.println("Initiating Request...");
    handleCP();
    Braccio.moveTo(interPos[0], interPos[1], interPos[2], interPos[3],
                   interPos[4], interPos[5]);
    if (monitorArm) {
      setMates(interPos[0], interPos[1], interPos[2], interPos[3], interPos[4],
               interPos[5]);
    } else {
      delay(2000);
    }

    float prevCP[6] = {interPos[0], interPos[1], interPos[2],
                       interPos[3], interPos[4], interPos[5]};

    for (int i = 0; i < cpNum; i++) {
      Serial.println(
          F("--------------------------------------------------------"));
      Serial.print(F("Checkpoint "));
      Serial.println(i + 1);
      Serial.println(
          F("--------------------------------------------------------"));
      Serial.print(F("X Pos: "));
      Serial.println(positions[i][0], 8);
      Serial.print(F("Y Pos: "));
      Serial.println(positions[i][1], 8);
      Serial.print(F("Z Pos: "));
      Serial.println(positions[i][2]), 8;
      Serial.println();
      bool gripperConf = false;
      if (positions[i][3] != 1) {
        gripperConf = true;
      }
      if (bracciocalc.calculateAngles(positions[i][0], positions[i][1],
                                      positions[i][2], gripperConf, angles)) {
        Serial.println("Angles Calculated & in Range");
        Serial.println();
        Serial.println("Gripper Angle: " + ((String)angles[0]));
        Serial.println("Wrist vert: " + ((String)angles[2]));
        Serial.println("Elbow: " + ((String)angles[3]));
        Serial.println("Shoulder: " + ((String)angles[4]));
        Serial.println("Base: " + ((String)angles[5]));

        Braccio.moveTo(prevCP[0], angles[1], angles[2], angles[3], angles[4],
                       angles[5]);
        delay(1000);
        Braccio.moveTo(angles[0], angles[1], angles[2], angles[3], angles[4],
                       angles[5]);
        if (monitorArm) {
          setMates(angles[0], angles[1], angles[2], angles[3], angles[4],
                   angles[5]);
        } else {
          delay(2000);
        }
        delay(positions[i][4] * 1000);
        for (int k = 0; k < 6; k++) {
          prevCP[k] = angles[k];
        }
      } else {
        Serial.println("Angles out of range - Checkpoint Skipped");
        Serial.println();
      }
    }

    Serial.println(
        F("--------------------------------------------------------"));
    Serial.println(F("Ready"));
  }
}

void handleCP() {
  char* outputString = new char[37000];
  myclient.handleChunkRequest(
      "GET",
      "https://rogers.onshape.com/api/assemblies/d/7ab185e141f1c90dd5141a87/w/"
      "ea910db75f797df5c9199171/e/536d55a7d85c8dd3e0eafd47",
      outputString);
  Serial.println("Got Response...");
  DynamicJsonDocument responseJson(37000);
  DeserializationError error = deserializeJson(responseJson, outputString);

  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }
  Serial.println(F("Parsed JSON, getting checkpoints..."));
  Serial.println();

  getCheckpoints(responseJson);
  responseJson.clear();
  delete[] outputString;
}

void getCheckpoints(DynamicJsonDocument& localDoc) {
  int arrIndex = 0;
  int instancesSize = localDoc["rootAssembly"]["instances"].size();
  String cpNames[10];

  for (int j = 0; j < instancesSize; j++) {
    String instName =
        localDoc["rootAssembly"]["instances"][j]["name"].as<String>();
    if (instName.indexOf("Checkpoint") != -1) {
      String configuration =
          localDoc["rootAssembly"]["instances"][j]["configuration"]
              .as<String>();
      char configChars[100];
      configuration.toCharArray(configChars, configuration.length() + 1);
      char* pch;
      pch = strtok(configChars, ";");
      char* gripperC = pch;
      pch = strtok(NULL, ";");
      char* delayC = pch;
      pch = strtok(NULL, ";");
      char* configC = pch;

      char configLastChar = configC[strlen(configC) - 1];
      int configNum = 0;
      if (isdigit(configLastChar)) {
        configNum = configLastChar - '0';
      }
      cpNames[configNum] =
          localDoc["rootAssembly"]["instances"][j]["id"].as<String>();
      positions[configNum][3] = 0;
      if (gripperC[strlen(gripperC) - 1] != 't') {
        positions[configNum][3] = 1;
      }
      positions[configNum][4] = 0;
      if (delayC[strlen(delayC) - 1] != 't') {
        positions[configNum][4] = delayC[strlen(delayC) - 1] - '0';
      }

      arrIndex++;
    }
  }

  for (int i = 0; i < localDoc["rootAssembly"]["occurrences"].size(); i++) {
    for (int j = 0; j < arrIndex; j++) {
      if (localDoc["rootAssembly"]["occurrences"][i]["path"][0].as<String>() ==
          cpNames[j]) {
        positions[j][0] =
            localDoc["rootAssembly"]["occurrences"][i]["transform"][3];
        positions[j][1] =
            localDoc["rootAssembly"]["occurrences"][i]["transform"][7];
        positions[j][2] =
            localDoc["rootAssembly"]["occurrences"][i]["transform"][11];
      }
    }
  }
  Serial.print(F("Total Checkpoints = "));
  Serial.println(arrIndex);
  cpNum = arrIndex;
}

void setMates(float gripper, float wristRoll, float wristPitch, float elbow,
              float shoulder, float base) {
  DynamicJsonDocument mateVals(4000);
  char* responsePtr = myclient.makeRequest(
      "GET",
      "https://rogers.onshape.com/api/assemblies/d/7ab185e141f1c90dd5141a87/w/"
      "ea910db75f797df5c9199171/e/536d55a7d85c8dd3e0eafd47/matevalues");

  DeserializationError error = deserializeJson(mateVals, responsePtr);

  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }

  DynamicJsonDocument mates(1024);
  JsonArray matesroot = mates.createNestedArray("mateValues");

  bool gripperClosed = false;
  if (gripper != homePos[0]) {
    gripperClosed = true;
  }

  for (int i = 0; i < mateVals["mateValues"].size(); i++) {
    if (mateVals["mateValues"][i]["mateName"].as<String>() == "Base_Revolute") {
      mateVals["mateValues"][i]["rotationZ"] = (-base + 180) * (0.017453);
      matesroot.add(mateVals["mateValues"][i]);
    } else if (mateVals["mateValues"][i]["mateName"].as<String>() ==
               "Shoulder_Revolute") {
      mateVals["mateValues"][i]["rotationZ"] =
          (shoulder - homePos[4]) * (0.017453);
      matesroot.add(mateVals["mateValues"][i]);

    } else if (mateVals["mateValues"][i]["mateName"].as<String>() ==
               "Elbow_Revolute") {
      mateVals["mateValues"][i]["rotationZ"] =
          (elbow - homePos[3]) * (0.017453);
      matesroot.add(mateVals["mateValues"][i]);

    } else if (mateVals["mateValues"][i]["mateName"].as<String>() ==
               "WristPitch_Revolute") {
      mateVals["mateValues"][i]["rotationZ"] =
          (wristPitch - homePos[2]) * (0.017453);
      matesroot.add(mateVals["mateValues"][i]);

    } else if (mateVals["mateValues"][i]["mateName"].as<String>() ==
               "WristRoll_Revolute") {
      mateVals["mateValues"][i]["rotationZ"] =
          (wristRoll - homePos[1]) * (0.017453);
      matesroot.add(mateVals["mateValues"][i]);

    } else if (mateVals["mateValues"][i]["mateName"].as<String>() ==
               "Claw_Revolute") {
      mateVals["mateValues"][i]["rotationZ"] = -45 * (0.017453);
      if (gripperClosed) {
        mateVals["mateValues"][i]["rotationZ"] = -30 * (0.017453);
      }
      matesroot.add(mateVals["mateValues"][i]);
    } else if (mateVals["mateValues"][i]["mateName"].as<String>() ==
               "Idler_Revolute") {
      mateVals["mateValues"][i]["rotationZ"] = (60) * 0.017453;
      if (gripperClosed) {
        mateVals["mateValues"][i]["rotationZ"] = (-5) * 0.017453;
      }
      matesroot.add(mateVals["mateValues"][i]);
    }
  }
  mateVals.clear();
  char matesString[1000];
  serializeJson(mates, matesString);
  matesroot.clear();
  mates.clear();

  myclient.makeRequest(
      "POST",
      "https://rogers.onshape.com/api/assemblies/d/7ab185e141f1c90dd5141a87/w/"
      "ea910db75f797df5c9199171/e/536d55a7d85c8dd3e0eafd47/matevalues",
      matesString);
  Serial.println("Successfully set mates!");
}
