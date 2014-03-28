package de.dlr.sc.tiglviewer.android;

public class ChildItem extends Item {
    
    public String source;
    
    public ChildItem(String name, String source)
    {
        this.name = name;
        this.source = source;
    }
    @Override
    public int isParentSection() {
        return 0;
    }
}
