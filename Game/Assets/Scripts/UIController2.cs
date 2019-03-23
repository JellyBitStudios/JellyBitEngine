using System.Collections;
using JellyBitEngine;
using JellyBitEngine.UI;

public class UIController2 : JellyScript
{
    // Alita
    public GameObject alita = null;
    private Alita alita_controller = null;

    // Attributes
    private int vitality = 0;
    private int strength = 0;
    private int dexterity = 0;
    private int armor = 0;
    private int crit_c = 0;
    private int crit_d = 0;

    // Alita Life Bar
    public GameObject life_bar = null;
    private RectTransform life_bar_rect = null;
    private uint life_bar_width = 0;

    // Focus
    public GameObject focus = null;
    public GameObject focus_background = null;
    private RectTransform focus_rect = null;
    private uint focus_width = 0;

    // SKills CDR
    public GameObject q_cdr = null;
    private RectTransform q_cdr_rect = null;
    private uint skill_height = 0;
    private uint skill_y = 0;
    private float cdr = 0.5f;

    // Holder
    public GameObject holder = null;
    public GameObject holder_item = null;
    private RectTransform holder_rect = null;
    private RectTransform holder_item_rect = null;
    private Item holding_item;

    // Tooltip
    public GameObject tooltip = null;
    private RectTransform tooltip_rect = null;
    private int tooltip_index = -1;

    // Skill Tree
    public GameObject skill_tree = null;

    // Inventory
    public GameObject inventory = null;
    private Item[] equipment;
    private GameObject[] equip_gos = null;
    private Button[] equip_buttons = null;

    public GameObject eq0 = null;
    public GameObject eq1 = null;
    public GameObject eq2 = null;
    public GameObject eq3 = null;
    public GameObject eq4 = null;
    public GameObject eq5 = null;
    
    public override void Awake()
    {

    }

    public override void Update()
    {

    }
}

