plugin.id = "chat-g";

var LcdLib;
var LcdInit;
var LcdClose;
var LcdPrint;

plugin.init =
function _init(glob) {
	this.major = 0;  // Major version number.
	this.minor = 1;  // Minor version number.
	this.version = this.major + "." + this.minor;
	this.description = "G15 LCD display plugin";
	
	return true;
}

plugin.enable =
function _enable() {
	// Open the Chat-G15 library
	var path = decodeURI(plugin.cwd.substr(8)) + "g15-chat.dll"; //Substr to remove file:///
	Components.utils.import("resource://gre/modules/ctypes.jsm");
	client.display("Opening library: " + path);
	LcdLib = ctypes.open(path);
	LcdInit = LcdLib.declare("LcdInit", ctypes.default_abi, ctypes.int, ctypes.jschar.ptr); // Returns: int Params: w_char*
	LcdClose = LcdLib.declare("LcdClose", ctypes.default_abi, ctypes.void_t); // Returns: void Params: void
	LcdPrint = LcdLib.declare("LcdPrint", ctypes.default_abi, ctypes.void_t, ctypes.jschar.ptr); // Returns: void Params: w_char*
	
	// Connect to the LCD display
	var ret = LcdInit("ChatZilla");
	if(ret != 0) return false; // Failed to connect
	
	// Add our message hooks
	client.eventPump.addHook([{set:"channel", type:"privmsg"}], hookMessage, this.id + "-hook-message");
	client.eventPump.addHook([{set:"user", type:"privmsg"}], hookPrivate, this.id + "-hook-private");
	client.eventPump.addHook([{set:"channel", type:"action"}], hookAction, this.id + "-hook-action");
	client.eventPump.addHook([{set:"user", type:"action"}], hookAction, this.id + "-hook-privaction");
	
	return true;
}

plugin.disable =
function _disable() {
	// Remove our message hooks
	client.eventPump.removeHookByName(this.id + "-hook-message");
	client.eventPump.removeHookByName(this.id + "-hook-private");
	client.eventPump.removeHookByName(this.id + "-hook-action");
	client.eventPump.removeHookByName(this.id + "-hook-privaction");
	
	// Close the connection and the library
	// If these functions aren't called firefox will lock up
	LcdClose();
	LcdLib.close();
	
	return true;
}

function hookMessage(e) {
	try {
		LcdPrint("<" + e.user.unicodeName + ">" + e.msg);
	} catch(ex) {
		// We should never let exceptions get out of a hook, or bad things happen.
	}
}

function hookAction(e) {
	try {
		LcdPrint(e.user.unicodeName + " " + e.CTCPData);
	} catch(ex) {
		// We should never let exceptions get out of a hook, or bad things happen.
	}
}

function hookPrivate(e) {
	try {
		LcdPrint("{" + e.user.unicodeName + "}" + e.msg);
	} catch(ex) {
		// We should never let exceptions get out of a hook, or bad things happen.
	}
}
