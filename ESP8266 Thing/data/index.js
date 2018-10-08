var pages = new Array('one', 'two', 'three', 'four');
var numBalls = 10;
var numTime = 6;
var data = false;
var playerName = "";

//Ask for the leaderboard values once the page loads
window.onload = setLeaderboard();

//Interface helper functions
//Forked from Ian Turner under MIT
function toggleMenu() {
  document.getElementsByClassName('wrapper')[0].classList.toggle('menu-open');
}

function goToPage(page) {
  var wrapper = document.getElementsByClassName('wrapper')[0];
  var sections = document.getElementsByTagName('section');
  for (var i = 0; i < sections.length; i++) {
    sections[i].classList.remove('before', 'after');
    if (i > page) {
      sections[i].classList.add('after');
    }
  }
  wrapper.classList.remove('menu-open', 'page-one', 'page-two');
  wrapper.classList.add('page-' + pages[page]);
}

//Play tab helper functions
function ballChoice(choice) {
  numBalls = choice;
  document.getElementById('ballButtons').style.display = 'none';
  document.getElementById('timeButtons').style.display = 'block';
}

function timeChoice(choice) {
  numTime = choice;
  document.getElementById('timeButtons').style.display = 'none';
  document.getElementById('readyScreen').style.display = 'block';
}

//Sets player name and calls getStats()
//Called when user submits their username
function setName(boxID) {
  playerName = document.getElementById(boxID).value;
  console.log(playerName);
  if(getStats()){
    document.getElementById('playLogin').style.display = 'none';
    document.getElementById('statLogin').style.display = 'none';
    document.getElementById('ballButtons').style.display = 'block';
    document.getElementById('statScreen').style.display = 'block';
  }
}

//Called on load of pages
//Queries the server for the leaderboard and receives them as a string
function setLeaderboard(){
  var client = new XMLHttpRequest();
  client.open('GET', 'leader', true);
  client.onreadystatechange = function() {
    if (client.readyState == 4){
			if(client.status == 200) {
        var leaders = client.responseText;
        console.log(leaders);
        var leadArr = CSVtoArray(leaders.slice(0,-3)); //parse the return string minus the extraneous line ending and comma
        console.log(leadArr);
        var leadObjs = [];
        //Create a sortable array of objects
        for(var i = 0; i < leadArr.length; i += 2){
          leadObjs.push({'name': leadArr[i], 'score': parseInt(leadArr[i+1])});
        }
        console.log(leadObjs);
        leadObjs = leadObjs.sort(function(a, b){ return b.score - a.score; });
        console.log(leadObjs);
        //Add the elements to the leaderboard
        for(var i = 0; i < leadObjs.length; i++){
          var newLi = document.createElement('li');
          var playerSpan = document.createElement('span');
          playerSpan.className = "name";
          playerSpan.innerHTML = leadObjs[i].name;
          var scoreSpan = document.createElement('span');
          scoreSpan.className = "percent";
          scoreSpan.innerHTML = leadObjs[i].score.toString();
          newLi.appendChild(playerSpan);
          newLi.appendChild(scoreSpan);
          document.getElementById('leaderList').appendChild(newLi);
        }
      }
    }
  }
  client.send();
}

// Return array of string values, or NULL if CSV string not well formed.
function CSVtoArray(text) {
    var re_valid = /^\s*(?:'[^'\\]*(?:\\[\S\s][^'\\]*)*'|"[^"\\]*(?:\\[\S\s][^"\\]*)*"|[^,'"\s\\]*(?:\s+[^,'"\s\\]+)*)\s*(?:,\s*(?:'[^'\\]*(?:\\[\S\s][^'\\]*)*'|"[^"\\]*(?:\\[\S\s][^"\\]*)*"|[^,'"\s\\]*(?:\s+[^,'"\s\\]+)*)\s*)*$/;
    var re_value = /(?!\s*$)\s*(?:'([^'\\]*(?:\\[\S\s][^'\\]*)*)'|"([^"\\]*(?:\\[\S\s][^"\\]*)*)"|([^,'"\s\\]*(?:\s+[^,'"\s\\]+)*))\s*(?:,|$)/g;
    // Return NULL if input string is not well formed CSV string.
    if (!re_valid.test(text)) return null;
    var a = [];                     // Initialize array to receive values.
    text.replace(re_value, // "Walk" the string using replace with callback.
        function(m0, m1, m2, m3) {
            // Remove backslash from \' in single quoted values.
            if      (m1 !== undefined) a.push(m1.replace(/\\'/g, "'"));
            // Remove backslash from \" in double quoted values.
            else if (m2 !== undefined) a.push(m2.replace(/\\"/g, '"'));
            else if (m3 !== undefined) a.push(m3);
            return ''; // Return empty string.
        });
    // Handle special case of empty last value.
    if (/,\s*$/.test(text)) a.push('');
    return a;
};

//Sends name to server and sets stats
//If they don't exist, prompts to create a profile
function getStats() {
  var nocache = "&nocache=" + Math.random() *10001;
	//var uri = "json.json";
  var uri = "stats" + "?name=" + playerName + nocache;
  console.log(uri);
	var xh = new XMLHttpRequest();
  xh.overrideMimeType("text/json");
	xh.onreadystatechange = function(){
		if (xh.readyState == 4){
      console.log("Succesful load");
			if(xh.status == 200) {
				if (this.responseText != null) {
          var res = xh.responseText;
          console.log("response is " + res);
          var res = String(xh.responseText);
          document.getElementsByClassName("insertName")[0].innerHTML += playerName;
          document.getElementsByClassName("insertName")[1].innerHTML += playerName;
          document.getElementById("bestScore").innerHTML += res.slice(0,2);
          document.getElementById("lastTry").innerHTML += res.slice(2,4);
          document.getElementById("totalNum").innerHTML += (res.length - 4)/2;
          var vals = [];
          for(var i = 4; i < res.length; i += 2){
            vals.push(parseInt(res.slice(i, i+2)));
          }
          var sumVals = vals.reduce(function(acc, val) { return acc + val; });
          document.getElementById("totalPercent").innerHTML += Math.ceil(sumVals/vals.length);
				}
        return true;
			}else if(xh.status = 201){
        console.log("Player not found");
        if(confirm('Player not found. Create their profile?')){
          createPlayer();
          return false;
        }else{
          return false;
        }
      }
		}
	};
  xh.open("GET", uri, true);
	xh.send(null);
  return true;
}

//Creates a new player for the userFile
//Called from inside getStats() if user agrees to create a new profile
function createPlayer(){
  var nocache = "&nocache=" + Math.random() *10001;
	//var uri = "json.json";
  var uri = "create" + "?name=" + String(playerName) + nocache;
  console.log(uri);
	var xhr = new XMLHttpRequest();
	xhr.onreadystatechange = function(){
		if (xhr.readyState == 4){
			if(xhr.status == 200) {
				if (this.responseText != null) {
					console.log(this.responseText);
          document.getElementById('newScreen').style.display = 'block';
          document.getElementById('statScreen').style.display = 'none';
          document.getElementById('playLogin').style.display = 'none';
          document.getElementById('statLogin').style.display = 'none';
          document.getElementById('ballButtons').style.display = 'block';
				}
			}
		}
	};
	xhr.overrideMimeType("text/json");
	//console.log(uri);
	xhr.open("GET", uri, true);
	xhr.send(null);
}

//Sends player name and parameters to server and starts the drill
function startDrill(){
  var nocache = "&nocache=" + Math.random() *10001;
	//var uri = "json.json";
  var uri = "start" + "?name=" + String(playerName) + "&balls=" + String(numBalls) + "&touchT=" + String(numTime) + nocache;
  console.log(uri);
	var xh = new XMLHttpRequest();
	xh.onreadystatechange = function(){
		if (xh.readyState == 4){
			if (this.responseText != null) {
        console.log(this.responseText);
        document.getElementById('readyScreen').style.display = 'none';
				document.getElementById('countDown').style.display = 'block';
			}
		}
	};
	xh.overrideMimeType("text/json");
	//console.log(uri);
	xh.open("GET", uri, true);
	xh.send(null);
}
