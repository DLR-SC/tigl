function Point3d(x,y,z) {
    this.x = x;
    this.y = y;
    this.z = z;
}

Point3d.prototype.toString = function() {
    return 'Point3d(' + this.x + ',' + this.y + ',' + this.z + ')'; 
}

Point3d.prototype.mult = function(f) {
    return new Point3d(f * this.x, f * this.y, f * this.z);
}

Point3d.prototype.dot = function(p) {
    return p.x * this.x + p.y * this.y + p.z * this.z;
}

Point3d.prototype.length = function() {
    return Math.sqrt(this.dot(this))
}

Point3d.prototype.normalized = function()
{
    return this.mult(1./this.length());
}

Point3d.prototype.add = function(p) {
    return new Point3d(this.x + p.x, this.y + p.y, this.z + p.z);
}

Point3d.prototype.subtract = function(p) {
    return new Point3d(this.x - p.x, this.y - p.y, this.z - p.z);
}

Point3d.prototype.cross = function(p) {
    return new Point3d(this.y*p.z - this.z*p.y, this.z*p.x - this.x*p.z, this.x*p.y - this.y*p.x);
}

Point3d.help = function() {
    s =  "Point3d method help:\n";
    s += "    mult(factor): scales the point\n";
    s += "    add(Point3d): addition of points\n";
    s += "    subtract(Point3d): subtraction of points\n";
    s += "    dot(Point3d): scalar product\n";
    s += "    length(): length of vector\n";
    s += "    normalized(): returns normalized vector (length=1)\n";
    s += "    cross(Point3d): cross product\n";
    s += "\n";
    return s
}

function GetPointDirectionResult(point, distance) {
    this.point = point;
    this.errorDistance = distance;
}

GetPointDirectionResult.prototype.toString = function() {
    return '{ point: ' + this.point + ', errorDistance = ' + this.errorDistance + ' }'; 
}

GetPointDirectionResult.prototype.getPoint = function() {
    return this.point;
}

GetPointDirectionResult.prototype.getErrorDistance = function() {
    return this.errorDistance;
}

function drawPoint() {
    if (arguments.length == 1) {
        p = arguments[0]
        if (typeof p === 'object' && p.constructor.name == 'Point3d') {
            app.scene.drawPoint(p.x, p.y, p.z);
        }
        else {
            throw Error("Invalid argument. Argument type must be Point3d.");
        }
    }
    else if (arguments.length == 3) {
        app.scene.drawPoint(arguments[0], arguments[1], arguments[2])
    }
    else {
        throw Error("Invalid argument count. Valid possibilities are: drawPoint(Point3d), drawPoint(x,y,z)");
    }
}

function drawVector() {
    if (arguments.length == 2) {
        p = arguments[0]
        n = arguments[1]
        if (typeof p === 'object' && p.constructor.name == 'Point3d' && typeof n == 'object' && n.constructor.name == 'Point3d') {
            app.scene.drawVector(p.x, p.y, p.z, n.x, n.y, n.z);
        }
        else {
            throw Error("Invalid argument. Argument type must be Point3d.");
        }
    }
    else if (arguments.length == 6) {
        app.scene.drawVector(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5])
    }
    else {
        throw Error("Invalid argument count. Valid possibilities are: drawVector(Point3d, Point3d), drawVector(x,y,z, dirx, diry, dirz)");
    }
}

function drawShape(shape) {
    app.scene.displayShape(shape)
}

function _getObjectMethods(obj) {
    var res = [];
    for (meth in obj) {
        if (!meth.startsWith("_")) {
            res.push(meth);
        }
    }
    return res;
}

function help(arg, derived) {
    if (_isQObject(arg)) {
        meta = _qobjGetMetaObj(arg);
        help(meta);

    }
    else if (_isQMetaObject(arg)) {
        if (derived) {
            if (_qMetaObjDescription(arg) == "QObject" ) {
                return;
            }
            print("Derived from <b>" + _qMetaObjDescription(arg) + "</b> with the methods:\n")
        }
        else {
            print("The <b>" + _qMetaObjDescription(arg) + "</b> class provides the following methods:\n")
        }
        members = _qMetaObjMembers(arg);
        props = _qMetaObjProperties(arg);
        for (i in members) {
           print("    " + members[i]);
        }
        if (props.length > 0) {
            if (derived) {
                print("\nAnd properties:\n");
            }
            else {
                print("\nThe following properties are defined:\n");
            }
            for (i in props) {
                print("    " + props[i]);
            }
        }
        sup = _qMetaObjGetSuperclass(arg);
        if (sup) {
            print("\n");
            help(sup, true);
        }
    }
    else if (typeof arg == "object") {
        print("This object provides following methods:\n");
        methods = _getObjectMethods(arg);
        for(var i in methods) {
            mname = methods[i];
            meth = arg[mname];
            if (typeof meth == "function" && !_isQMetaObject(meth)) {
                print("    " + mname);
            }
        }
    }
    else {
        print("type: " + typeof(arg))
    }
}

// add some some convenience function
if (typeof String.prototype.startsWith != 'function') {
  // see below for better implementation!
  String.prototype.startsWith = function (str){
    return this.indexOf(str) == 0;
  };
}
