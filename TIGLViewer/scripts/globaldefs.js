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

function help(arg) {
    if (typeof arg == "object") {
        print(arg + ":\n");
        for(var m in arg) {
            if(typeof arg[m] == "function") {
                print("    " + m);
            }
        }
    }
    else {
        print("type: " + typeof(arg))
    }
}
