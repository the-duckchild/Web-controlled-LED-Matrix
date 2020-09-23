#include <ht16k33.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <SPI.h>



/* WIFI DETAILS HERE..........................................*/

ESP8266WebServer server;
char* ssid = "EE-Hub-Z9Sc";
char* password = "ELM-remit-ever";

/* HTML DOC saved to Flash...................................................*/

char webpage[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="en-gb">

<head>

    <script src="https://ajax.googleapis.com/ajax/libs/jquery/3.3.1/jquery.min.js"></script>

    <style>
        body {
            display: grid;
            align-content: center;
            justify-content: center;
        }
        
        #container {
            display: grid;
            grid-template-columns: repeat(8, auto);
            grid-template-rows: repeat(12, auto);
            height: 95vh;
            width: 95vw;
            align-items: stretch;
            justify-items: stretch;
            row-gap: 15px;
            column-gap: 15px;
            grid-auto-flow: column;
        }
        
        #container label {
            background: #21618C;
            border-radius: 3000px;
            width: 100%;
            height: 100%;
            display: block;
        }
        
        input[type="checkbox"] {
            display: none;
        }
        
        #container input[type="checkbox"]:checked~label {
            background: red;
        }
    </style>

    <title>Twig's LED Climbing Wall</title>
</head>


<body>

    <div id="container" class="controls">
        <!-- 96 buttons IDs to match array positions numbered by LED grid -->
        <script>
            //define initial LED array

            let ledArray = [];
            ledArray.length = 128;
            ledArray.fill(0, 0, 128); // use the fill method to avoid needing longhand assignment of lots of zeroes
            console.log(ledArray);

            const $controls = document.getElementsByClassName("controls")[0];

            $controls.addEventListener("click", e => {
                // event handler on body - need to check if the orginal target of the
                // bubbled event has a value, i.e. is a checkbox
                const val = e.target.value;
                if (val) {
                    // this line gets all the checked values in one lump
                    //const $checked = document.querySelectorAll("input:checked");
                    // original code only mutates one item at a time which is cleaner
                    // so...
                    console.log(e.target, e.target.value, e.target.checked);
                    let arrayIndex = parseInt(e.target.value, 10);
                    ledArray[arrayIndex] = e.target.checked ? 1 : 0;
                    // ternary short-hand assignment
                    $.ajax({
                        type: "POST",
                        url: '/array',
                        data: ledArray.join(','),
                                            });
                    console.log(ledArray);

                }
            });

            // build the buttons
            const numRow = 12;
            const numColumn = 0;
            const bodyEl = document.querySelector('body')
            for (let numColumn = 0; numColumn < 8; numColumn++)
                for (let i = 0; i < numRow; i++) {
                    const startNum = (16 * numColumn)
                    const identifier = i + startNum;
                    const item = document.createElement('div')
                    item.class = "buttondiv"

                    const labelEl = document.createElement('label')
                    labelEl.htmlFor = identifier;

                    const checkBox = document.createElement('input')
                    checkBox.type = 'checkbox'
                    checkBox.id = identifier
                    checkBox.value = identifier

                    item.appendChild(checkBox);
                    item.appendChild(labelEl);

                    const container = document.getElementById('container')
                    container.appendChild(item)
                }
        </script>
    </div>
</body>

</html>
)=====";

/* sET UP WIFI CONNECTION AND PRINT IP ADDRESS................*/

//define the LED driver and use library

HT16K33 HT;

//define the start state of the array to control the LEDS

uint8_t ledArray [] = 
{1,1,0,0,0,0,1,1,1,1,1,1,0,0,0,0,
 1,1,0,0,0,0,1,1,1,1,1,1,0,0,0,0,
 1,1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,
 1,1,1,1,1,1,0,0,1,1,1,1,0,0,0,0,
 1,1,1,1,1,1,0,0,1,1,1,1,0,0,0,0,
 1,1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,
 1,1,0,0,0,0,1,1,1,1,1,1,0,0,0,0,
 1,1,0,0,0,0,1,1,1,1,1,1,0,0,0,0};

void updateArray ()
{
      String newArrayState =  server.arg("plain");
    
    
      
      int newArrayLength = newArrayState.length()+1;
      char newCharArray[newArrayLength];
      newArrayState.toCharArray(newCharArray,newArrayLength);
      server.send(200, "text/plain");
      //change String to char array
      
      int intArray[128];
      int ipos = 0;
      char *tok = strtok(newCharArray, ",");
      while (tok) {
        if (ipos <128) {
          intArray[ipos++] = atoi(tok);
        }
        tok = strtok(NULL, ",");
      }
      //converting char array to in array
      
      int i;
      for (i=0; i<128; i++)
  {
  ledArray[i] = intArray[i];
  }
    // copy new int array into ledArray to control LEDs
      }

void handleNotFound(){
  server.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}
void setup()
{


  WiFi.begin(ssid,password);
  Serial.begin(115200);
  while(WiFi.status()!=WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.on("/",[](){server.send_P(200,"text/html", webpage);});
  server.on("/array",HTTP_POST, updateArray); 
  
            
  
  server.begin();
  
  HT.begin(0x00);
  //START UP ANIMATION GOES HERE

}

void loop()
{

//read the LED array 
  uint8_t led;
  for (led=0; led<128; led++) 
  {
  if (ledArray[led] > 0)
   { HT.setLedNow(led);}
   else {HT.clearLedNow(led);}}
  

//Handle html requests   
  server.handleClient();
  
}
