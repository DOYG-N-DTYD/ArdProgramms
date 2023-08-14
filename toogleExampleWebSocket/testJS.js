let player = 1; // turn of player 1-X or 2-O 

document.getElementById('start').addEventListener('click', startGame);
document.getElementById('start').style.color = "green";

function initButtons() {
  for (let i = 0; i < 3; i += 1) {
    for (let j = 0; j < 3; j += 1) {
      let buttonIndex = 'button-'.concat(i.toString() + j.toString());
      document.getElementById(buttonIndex).addEventListener('click', gameClick);
      document.getElementById(buttonIndex).style.color = "#0f8b8d";
    }
  }
}

function drawGameBoard() {
  document.getElementById('start').style.color = "green";
  for (let i = 0; i < 3; i += 1) {
    for (let j = 0; j < 3; j += 1) {
      let buttonIndex = "button-".concat(i.toString() + j.toString());
      let button = document.getElementById(buttonIndex);
      button.innerHTML = i.toString() + j.toString();
      button.style.color = "#fff";
      button.style.opacity = "1";
    }
  }
}

function startGame() {
  document.getElementById('start').innerHTML = "Game Started";
  document.getElementById('start').style.color = "red";
  initButtons();
  drawGameBoard();
  checkWinConditions();
}

function gameClick() {
console.log("GAMECLICK");
  let field = document.getElementById(this.id);
  if (player == 1) {
    field.innerHTML = 'X';
    field.style.color = "red";
    player = 2;
  } else {
    field.innerHTML = 'O';
    field.style.color = "blue";
    player = 1;
  }
  checkWinConditions();
  if (checkifFull()) {
  document.getElementById("info_1").innerHTML = "TIE"
  return "TIE"};
}

let winCombinations = [
  "00,01,02",
  "10,11,12",
  "20,21,22",
  "00,10,20",
  "01,11,21",
  "02,12,22",
  "00,11,22",
  "02,11,20"
];

function checkWinConditions() {
console.log("CHECK WIN CONDITIONS");
  let countOfElementsInLine = 0;
  let playerSymbol;
  player == 1 ? playerSymbol = "O" : playerSymbol = "X";
  winCombinations.forEach((lineOfConditions) => {
    lineOfConditions.split(",").forEach((buttonIndex) => {
      let button = document.getElementById("button-" + buttonIndex);
      if (button.textContent == playerSymbol) {
        countOfElementsInLine += 1;
      }
      if (countOfElementsInLine == 3) {
      document.getElementById("info_1").innerHTML = "WINNING player " + playerSymbol;
        paintWinnerPositions(lineOfConditions)
        startGame();
      }
    });
    countOfElementsInLine = 0;
  });
}

function checkifFull() {
  let counterXO = 0;
  for (let i = 0; i < 3; i += 1) {
    for (let j = 0; j < 3; j += 1) {
      let buttonIndex = "button-".concat(i.toString() + j.toString());
      let button = document.getElementById(buttonIndex);
      if (button.innerHTML == "X" || button.innerHTML == "O") {
        counterXO += 1;
      }
    }
  }
  if (counterXO == 9) {
   document.getElementById("info_1").innerHTML = "TIE";
    startGame();
  } else {
    counterXO = 0;
  }
}

function paintWinnerPositions(lineOfConditions){
	 lineOfConditions.split(",").forEach((buttonIndex) => {
   	let button = document.getElementById("button-" + buttonIndex);
    button.style.color = "black";
    button.style.opacity = "0.5";
   });
}
