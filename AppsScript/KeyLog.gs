//Document information
var ss = SpreadsheetApp.openById('156XTsXdEbooZrMWVUeLs61gGUYXY3AJG2Iktad2Gya8');
var sheet = ss.getSheetByName('KeyLog');
var timezone = "America/Mexico_City";

function getFirstEmptyRow() {
  var column = sheet.getRange('E2:E');
  var values = column.getValues();
  var row = 0;
  while ( values[row] && values[row][0] != "" ) {
    row++;
  }
  return (row+2);
}

function doGet(e){
  Logger.log(JSON.stringify(e));

  //Get Data from ESP32
  if (e.parameter == 'undefined') {
    return ContentService.createTextOutput("Received data is undefined");
  }

  //Variable Declaration
  var EntryDate = new Date();
  var EntryTime = Utilities.formatDate(EntryDate, timezone, 'HH:mm:ss');
  var uid = e.parameters.uid.toString().replace(/^["']|['"]$/g, "");
  
  if (uid == -1) {

    var EnextRow = getFirstEmptyRow();

    sheet.getRange("E" + EnextRow).setValue(EntryTime);
      
    //returns response back to ESP32
    return ContentService.createTextOutput("Exit registered successfully!");

  } else {

    //Columns and rows arrangment
    var nextRow = sheet.getLastRow() + 1;
    var name = "VLOOKUP("+"A"+nextRow+";AccessReg!$A$1:$B$4;2;FALSE)";

    sheet.getRange("A" + nextRow).setValue(uid);
    sheet.getRange("B" + nextRow).setFormula(name);
    sheet.getRange("C" + nextRow).setValue(EntryDate);
    sheet.getRange("D" + nextRow).setValue(EntryTime);

    //returns response back to ESP32
    return ContentService.createTextOutput("Entry registered successfully!");
  }

}
