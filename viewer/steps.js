const ARMY_WIDTH = 3;
const HALF_BOARD_WIDTH = ARMY_WIDTH + 2;
const FULL_BOARD_WIDTH = 2* HALF_BOARD_WIDTH;

function discard(name){
	let allResources = [...resourceLists[iActive].children];
	let allSuitableResources = allResources.filter( el => (el.textContent.toLowerCase() == name.toLowerCase() ) );
	allSuitableResources[0].remove();
}

function draw(name, who){
	let resource = document.createElement('li');
	resource.textContent = name;
	resourceLists[who].appendChild(resource);
}

function teamClass(iTeam){
	return 'teamid-' + teamNames[iTeam].toLowerCase().replaceAll(' ', '-')
}

function characterFactory(char, iTeam){
	var charDom = document.createElement('div');
	charDom.id = 'cid-' + char.cid;
	charDom.classList.add( teamClass(iTeam) );
	charDom.classList.add('character');
	charDom.classList.add('team-' + (iTeam+1));
	charDom.innerHTML = 
		`<h1>${char.name}</h1>
		 <div class="stats">
		 <div class="HP">${char.HP}</div><div class="maxHP">${char.maxHP}</div>`+
		`<div class="atk softAtk">${char.softAtk}</div><div class="atk hardAtk">${char.hardAtk}</div><div class="mov">${char.mov}</div><div class="rng">${char.rng}</div>`+
		`<div class="netWorth">${char.netWorth}</div><i>${char.flavor}</i>`;
	return charDom;
}

function bluePrintCharacter(charDom){
	let stats = charDom.children[1];
	return {
		HP : Number( stats.children[0].textContent ),
		maxHP : Number( stats.children[1].textContent ),
		softAtk : Number( stats.children[2].textContent ),
		hardAtk : Number( stats.children[3].textContent ),
		mov : Number( stats.children[4].textContent ),
		rng : Number( stats.children[5].textContent ),
		netWorth : Number( stats.children[6].textContent ),
		flavor : stats.children[7].textContent
	};
}

function getBoardField(position){
	return boardDom.children[ position[1] + FULL_BOARD_WIDTH * position[0] ];
}

function getNeighbor(pos, direction){
	switch(direction.toLowerCase()){
		case "right": return [ pos[0], pos[1] + 1 ];
		case "left":  return [ pos[0], pos[1] - 1 ];
		case "down":  return [ 1,      pos[1] ];
		case "up": return [ 0,      pos[1] ];
	}
	console.error('Unrecognized direction: ' + direction);
}

function inverse(direction){
	switch(direction.toLowerCase()){
		case "right": return "left";
		case "left": return "right";
		case "down": return "up";
		case "up": return "down";
	}
	console.error('Unrecognized direction: ' + direction);
}

function move(step, backwards) {
	let newPosition = step.frm;
	let iMove;
	if(!backwards){
		// the steps until the first COF just pass through the fields without changing anything
		for(iMove = 0; iMove < step.firstCOF; iMove++) {
			newPosition = getNeighbor( newPosition, step.moves[iMove] );
		}
		// The last step before COF should be an empty field
		if(getBoardField(newPosition).hasChildNodes()) console.error('Position ' + newPosition + ' not empty during COF');
		let previousPosition = newPosition;
		// The steps after the first COF should all contain units, which will be moved one field backwards
		for( ; iMove < step.moves.length; iMove++){
			newPosition = getNeighbor( newPosition, step.moves[iMove] );
			getBoardField(previousPosition).appendChild(getBoardField(newPosition).firstChild); //move unit one field backwards
		}
	} else { // do everything in the opposite direction
		let previousPosition = newPosition;
		for(iMove = step.moves.length - 1; iMove >= step.firstCOF; i--){
			newPosition = getNeighbor( newPosition, inverse(step.moves[i]) );
			getBoardField(previousPosition).appendChild(getBoardField(newPosition).firstChild);
		}

		if(getBoardField(newPosition).hasChildNodes()) console.error('Position ' + newPosition + ' not empty during COF');

		for(iMove = 0; iMove < step.firstCOF; i++) {
			newPosition = getNeighbor( newPosition, inverse(step.moves[i]) );
		}
	}
	// we're there!
	let unit = getBoardField(step.frm).firstChild;
	if(unit.id != 'cid-' + step.target) console.warn('Target is named ' + unit.id + ' instead of cid-' + step.target);
	if(newPosition[0] != step.to[0] || newPosition[1] != step.to[1]) console.error('Arrived at ' + newPosition + ' instead of ' + step.to);
	getBoardField(newPosition).appendChild(unit); //move the unit
	return { 'action' : 'move', 'frm' : step.to, 'to' : step.frm, 'target' : step.target, moves: step.moves, 'firstCOF' : step.firstCOF };
}

function apply(step, backwards) {
	console.log(step);
	switch(step.action){
		case 'beginTurn':
			iActive = 1 - iActive;
			return { 'action' : 'beginTurn' };
		case 'pass': return { 'action' : 'pass' };
		case 'move': {
			return move(step, backwards);
		}
		case 'draw': {
			let textContent = step.clss + ' - ' + step.value;
			if(!backwards) draw(textContent, iActive);
			else discard(textContent);
			return step;
		}
		case 'atk': {
			if(!backwards){
				discard('action - ' + step.cardLost);
				let attacker = boardDom.children[ step.subject[1] + FULL_BOARD_WIDTH*step.object[0] ].firstChild;
				let victim = boardDom.children[ step.object[1] + FULL_BOARD_WIDTH*step.object[0] ].firstChild;
				document.getElementById(attacker.id).classList.add("img");
				document.getElementById(victim.id).classList.add("img");
				victim.children[1].children[0].textContent = step.setLife;
				if(step.delete){
					step.deleted = bluePrintCharacter(victim);
					victim.remove();
				} else {
					setTimeout(function() {
						document.getElementById(victim.id).classList.remove("img");
					}, 1000);
					document.getElementById(victim.id).children[1].children[0].classList.add("body");
					setTimeout(function() {
						document.getElementById(victim.id).children[1].children[0].classList.remove("body");
					}, 1000);
				}
				setTimeout(function() {
					document.getElementById(attacker.id).classList.remove("img");
				}, 1000);

				return step;
			} else {
				iActive = 1 - iActive;
				let charDom = undefined;
				if(step.delete){
					charDom = characterFactory(step.deleted, 1 - iActive);
					boardDom.children[ step.object[1] + FULL_BOARD_WIDTH*step.object[0] ].appendChild(charDom);
					delete step.deleted;
				} else {
					charDom = boardDom.children[ step.object[1] + FULL_BOARD_WIDTH*step.object[0] ].firstChild;
				}
				charDom.children[1].children[0].textContent = step.setLife + step.lostLife;
				draw('action - ' + step.cardLost, iActive);
				return step;
			}
		}
		case 'def': {
			let subject = boardDom.children[ step.subject[1] + FULL_BOARD_WIDTH*step.subject[0] ];
			if(!backwards){
				discard('action - ' + step.cardLost);
				subject.children[0].children[1].children[0].textContent = step.permanent;
				iActive = 1 - iActive;
			} else {
				iActive = 1 - iActive;
				subject.children[0].children[1].children[0].textContent = step.permanent - 50;
				draw('action - ' + step.cardLost, iActive);
			}
			return step;
		}
		default:
			console.error('Step type ' + step.action + ' is not implemented');
	}
}

var teamNames = [];
var boardDom = [];
var resourceLists = [];
var screen = null;
var iActive = 0;

function createState(teams, names, board, players){
	for(let team of teams)
		for(let row of team)
				if(!row.HP) row.HP = row.maxHP;

	let outerScreen = document.getElementById('screen');
	outerScreen.textContent = '';

	screen = document.createElement('div');
	screen.id = 'screen-inner';
	screen.style.cssText += 'width:100%;height:100%;display:flex;flex-direction:column;';
	outerScreen.appendChild(screen);

	boardDom = document.createElement('div');
	boardDom.id = 'board';
	boardDom.classList.add('board-style');
	screen.appendChild(boardDom);

	for(let i = 0; i < 2; i++){
		let row = [];
		for(let j = 0; j < FULL_BOARD_WIDTH; j++){
			let td = document.createElement('div');
			td.setAttribute('array', '[' + j + ', ' + i +']')
			td.classList.add('board-field');
			boardDom.appendChild(td);
			row.push(td);
		}
	}

	let headers = [];

	for(let i=0; i<2; i++){
		teamNames[i] = names[i];
		let header = document.createElement('div');
		header.innerHTML = `<h3>${teamNames[i]}</h3>`;
		let resourceList = document.createElement('ul');
		resourceList.classList.add('resource-list');
		console.log("WELCOME");
		resourceLists.push(resourceList);
		header.appendChild(resourceList);
		header.style.cssText += 'margin:1;'
		header.classList.add(teamClass(i));
		header.classList.add('header');
		header.classList.add('teamIndex-' + i);
		headers.push(header);

		for(let j=0; j<2; j++) {
			let team = teams[i];
			let pos = board[j];
			if (i == 0) {
				for (let k = 0; k < HALF_BOARD_WIDTH; k++) {
					let poss = pos[k];
					let charDom;
					if(poss != null) {
						let posChar = poss[1];
						charDom = characterFactory(team[posChar], i);
						boardDom.children[k + FULL_BOARD_WIDTH * j].appendChild(charDom);
					}
				}
			} else {
				for (let k = HALF_BOARD_WIDTH; k < FULL_BOARD_WIDTH; k++) {
					let charDom;
					let poss = pos[k];
					if(poss != null) {
						let posChar = poss[1];
						charDom = characterFactory(team[posChar], i);
						boardDom.children[k + FULL_BOARD_WIDTH * j].appendChild(charDom);
					}
				}
			}
		}
	}
	screen.prepend(headers[0]);
	screen.append(headers[1]);

	for(let iPlayer=0; iPlayer<2; iPlayer++){
		for(let card of players[iPlayer].actions) draw('action - ' + card, iPlayer);
		for(let card of players[iPlayer].resources) draw('resource - ' + card, iPlayer);
	}

	return null;
}

function readGame(content, socket){
	try {
		socket = socket;
		const ret = JSON.parse(content);
		let init = ret.state;
		let teams = ret.state.aliveUnits;
		let names = ret.state.teamNames;
		let board = ret.state.board;
		let state = createState(teams, names, board, init.players);

		return { state : state, steps : ret.steps }
	} catch(e) {
		return "No parsing";
	}
}

function readStep(content) {
	return JSON.parse(content);
}