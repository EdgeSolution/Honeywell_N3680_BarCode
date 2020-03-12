package honeywell.hedc;

import android.content.Context;
import android.view.View;
import android.widget.EditText;
import android.widget.Switch;

/**
 * Symbology configuration for setting symbology settings.
 *
 */
public class SymbologyConfig implements View.OnClickListener {

    public interface DeviceCmd{
        public int GetIntSetting(String cmd);
        public boolean SetIntSetting(String cmd);
    }

    private DeviceCmd device;
    public void setDevice(DeviceCmd dev) {
        this.device = dev;
    }

    /**
     * Symbology Config structure used for configuring symbologies.
     */
    public SymbologyConfig(String prefix, String postfix, Switch On) {
        this.setPrefix(prefix);
        this.setOn(On);
        this.setMaxLimit(0);
        this.setPostfixEnable(postfix);
        this.setClickListeners();
    }

    public SymbologyConfig(String prefix, Switch On) {
        this.setPrefix(prefix);
        this.setOn(On);
        this.setMaxLimit(0);
        this.setPostfixEnable("ENA");
        this.setClickListeners();
    }

    public SymbologyConfig(String prefix, int limit, Switch On, EditText min, EditText max) {
        this.setPrefix(prefix);
        this.setMaxLimit(limit);
        this.setOn(On);
        this.setEditMin(min);
        this.setEditMax(max);
        this.setPostfixEnable("ENA");
        this.setClickListeners();
    }

    private String Prefix;
    private String PostfixEnable;

    private int Flags;
    private int MinLength;
    private int MaxLength;
    private int MaxLimit;
    private Switch SwitchOn;
    private EditText EditMin;
    private EditText EditMax;

    public String getPostfixEnable() {
        return PostfixEnable;
    }

    public void setPostfixEnable(String postfixEnable) {
        PostfixEnable = postfixEnable;
    }

    private String getEnableCmd() {
        return this.getPrefix() + this.getPostfixEnable();
    }

    private String getMinCmd() {
        return this.getPrefix() + "MIN";
    }

    private String getMaxCmd() {
        return this.getPrefix() + "MAX";
    }

    public String getSetEnableCmd(){
        return getEnableCmd() + Integer.toString(getFlags());
    }

    public String getSetMinCmd(){
        return getMinCmd() + Integer.toString(getMinLength());
    }

    public String getSetMaxCmd(){
        return getMaxCmd() + Integer.toString(getMaxLength());
    }

    public void QueryDevice(){
        setFlags(device.GetIntSetting(getEnableCmd()));
        if (getMaxLimit() > 0){ // supports min and max at all?
            setMinLength(device.GetIntSetting(getMinCmd()));
            setMaxLength(device.GetIntSetting(getMaxCmd()));
        }
        UpdateUI();
    }

    public boolean ConfigDevice(){
        boolean success = device.SetIntSetting(getSetEnableCmd());
        if (getMaxLimit() > 0) { // supports min and max at all?
            success &= device.SetIntSetting(getSetMinCmd());
            success &= device.SetIntSetting(getSetMaxCmd());
        }
        return success;
    }

    /**
     * Logical OR of valid flags for the given symbology
     */
    public int getFlags() {
        return Flags;
    }

    public void setFlags(int flags) {
        Flags = flags;
    }

    /**
     * Minimum length for valid barcode string for this symbology
     */
    public int getMinLength() {
        return MinLength;
    }

    public void setMinLength(int minLength) {
        if ((minLength <= 0 || minLength > getMaxLimit())) {
            minLength = 1;
        }
        MinLength = minLength;
    }

    /**
     * Maximum length for valid barcode string for this symbology
     */
    public int getMaxLength() {
        return MaxLength;
    }

    public void setMaxLength(int maxLength) {
        if ((maxLength < 0 || maxLength > getMaxLimit())) {
            maxLength = getMaxLimit();
        }
        MaxLength = maxLength;
    }

    /**
     * Symbology ID
     */
    public String getPrefix() {
        return Prefix;
    }

    public void setPrefix(String prefix) {
        Prefix = prefix;
    }

    public Switch getOn() {
        return SwitchOn;
    }

    public void setOn(Switch on) {
        SwitchOn = on;
    }

    public EditText getEditMin() {
        return EditMin;
    }

    public void setEditMin(EditText editMin) {
        EditMin = editMin;
    }

    public EditText getEditMax() {
        return EditMax;
    }

    public void setEditMax(EditText editMax) {
        EditMax = editMax;
    }

    public int getMaxLimit() {
        return MaxLimit;
    }

    public void setMaxLimit(int maxLimit) {
        MaxLimit = maxLimit;
    }

    @Override
    public void onClick(View v) {
//        super.onClick(v);
        Flags = SwitchOn.isChecked() ? 1 : 0;
    }

    public void setClickListeners() {
        if(SwitchOn != null) {
            SwitchOn.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    Flags = SwitchOn.isChecked() ? 1 : 0;
                }
            });
        }

        if(EditMax != null) {
            EditMax.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    String txt = EditMax.getText().toString();
                    if ((txt != null) && (!txt.isEmpty())) {
                        setMaxLength(Integer.parseInt(txt));
                    }
                }
            });
        }

        if(EditMin != null) {
            EditMin.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    String txt = EditMin.getText().toString();
                    if ((txt != null) && (!txt.isEmpty())) {
                        setMinLength(Integer.parseInt(txt));
                    }
                }
            });
        }
    }

    public void UpdateUI() {
        if(SwitchOn != null) {
            SwitchOn.setChecked(this.getFlags()==1 ? true : false);
        }

        if(EditMin != null) {
            EditMin.setText(Integer.toString(getMinLength()));
        }

        if(EditMax != null) {
            EditMax.setText(Integer.toString(getMaxLength()));
        }
    }
}

