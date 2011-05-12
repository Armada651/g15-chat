plugin.id = "g15-chat";

var LcdLib;
var LcdInit;
var LcdClose;
var LcdPrint;

var ColorFilter = new RegExp("\\x1f|\\x02|\\x12|\\x0f|\\x16|\\x03(?:\\d{1,2}(?:,\\d{1,2})?)?", "g");

plugin.init =
function _init(glob) {
    this.major = 0;  // Major version number.
    this.minor = 1;  // Minor version number.
    this.version = this.major + "." + this.minor;
    this.description = "ChatZilla G15 Display Plugin";

    return true;
}

plugin.enable =
function _enable() {
    // Open the G15-Chat library
    Components.utils.import("resource://gre/modules/ctypes.jsm");
    var path = decodeURI(plugin.cwd.substr(8)) + "g15-chat.dll";
    LcdLib = ctypes.open(path); //Substr to remove file:///
    display("G15 Chat DLL loaded from " + path);
    LcdInit = LcdLib.declare("LcdInit", ctypes.default_abi, ctypes.int, ctypes.jschar.ptr); // Returns: int Params: wchar_t*
    LcdClose = LcdLib.declare("LcdClose", ctypes.default_abi, ctypes.int); // Returns: void Params: void
    LcdPrint = LcdLib.declare("LcdPrint", ctypes.default_abi, ctypes.int, ctypes.jschar.ptr); // Returns: void Params: wchar_t*

    // Connect to the LCD display
    var ret = LcdInit("ChatZilla");
    if (ret != 0) return false; // Failed to connect

    // Add our message hooks
    client.eventPump.addHook([{ set: "channel", type: "privmsg"}], hookMessage, this.id + "-hook-message");
    client.eventPump.addHook([{ set: "user", type: "privmsg"}], hookPrivate, this.id + "-hook-private");
    client.eventPump.addHook([{ set: "channel", type: "action"}], hookAction, this.id + "-hook-action");
    client.eventPump.addHook([{ set: "user", type: "action"}], hookAction, this.id + "-hook-privaction");

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
        LcdPrint("<" + e.user.unicodeName.replace(ColorFilter, "") + ">" + e.msg.replace(ColorFilter, ""));
    } catch (ex) {
        // We should never let exceptions get out of a hook, or bad things happen.
    }
}

function hookAction(e) {
    try {
        LcdPrint(e.user.unicodeName.replace(ColorFilter, "") + " " + e.CTCPData.replace(ColorFilter, ""));
    } catch (ex) {
        // We should never let exceptions get out of a hook, or bad things happen.
    }
}

function hookPrivate(e) {
    try {
        LcdPrint("{" + e.user.unicodeName.replace(ColorFilter, "") + "}" + e.msg.replace(ColorFilter, ""));
    } catch (ex) {
        // We should never let exceptions get out of a hook, or bad things happen.
    }
}
