using System.Collections;
using JellyBitEngine;
using JellyBitEngine.UI;

public struct Item
{
    public int id;
}

public class UIController : JellyScript
{
    // Alita
    public GameObject alita = null;
    private Alita alita_controller = null;

    /*/ Attributes
    private int vitality = 0;
    private int strength = 0;
    private int dexterity = 0;
    private int armor = 0;
    private int crit_c = 0;
    private int crit_d = 0;*/

    // Alita Life Bar
    public GameObject life_bar = null;
    private RectTransform life_bar_rect = null;
    private uint life_bar_width = 0;

    // Focus
    public GameObject focus_background = null;
    public GameObject focus = null;
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
    public GameObject tooltip_background = null;
    public GameObject tooltip = null;
    private RectTransform tooltip_background_rect = null;
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

    public GameObject inv00 = null;
    public GameObject inv01 = null;
    public GameObject inv02 = null;
    public GameObject inv03 = null;
    public GameObject inv04 = null;
    public GameObject inv05 = null;
    public GameObject inv10 = null;
    public GameObject inv11 = null;
    public GameObject inv12 = null;
    public GameObject inv13 = null;
    public GameObject inv14 = null;
    public GameObject inv15 = null;
    public GameObject inv20 = null;
    public GameObject inv21 = null;
    public GameObject inv22 = null;
    public GameObject inv23 = null;
    public GameObject inv24 = null;
    public GameObject inv25 = null;
    public GameObject inv30 = null;
    public GameObject inv31 = null;
    public GameObject inv32 = null;
    public GameObject inv33 = null;
    public GameObject inv34 = null;
    public GameObject inv35 = null;
    public GameObject inv40 = null;
    public GameObject inv41 = null;
    public GameObject inv42 = null;
    public GameObject inv43 = null;
    public GameObject inv44 = null;
    public GameObject inv45 = null;
    public GameObject inv50 = null;
    public GameObject inv51 = null;
    public GameObject inv52 = null;
    public GameObject inv53 = null;
    public GameObject inv54 = null;
    public GameObject inv55 = null;
    public GameObject inv60 = null;
    public GameObject inv61 = null;
    public GameObject inv62 = null;
    public GameObject inv63 = null;
    public GameObject inv64 = null;
    public GameObject inv65 = null;
    public GameObject inv70 = null;
    public GameObject inv71 = null;
    public GameObject inv72 = null;
    public GameObject inv73 = null;
    public GameObject inv74 = null;
    public GameObject inv75 = null;

    public override void Awake()
    {
        if (alita != null)
        {
            alita_controller = alita.GetComponent<Alita>();

            // Alita Life Bar
            if (life_bar != null)
            {
                life_bar_rect = life_bar.GetComponent<RectTransform>();
                life_bar_width = life_bar_rect.x_dist;
            }

            // Focus
            if (focus != null && focus_background != null)
            {
                focus.active = focus_background.active = false;
                focus_rect = focus.GetComponent<RectTransform>();
                focus_width = focus_rect.x_dist;
            }

            // Skill
            if (q_cdr != null)
            {
                q_cdr_rect = q_cdr.GetComponent<RectTransform>();
                skill_height = q_cdr_rect.y_dist;
                skill_y = q_cdr_rect.y;
            }
        }
        // Skill Tree
        if (skill_tree != null)
            skill_tree.active = false;

        // Inventory
        if (eq0 != null)
        {
            inventory.active = false;

            equip_gos = new GameObject[] {
                eq0, eq1, eq2, eq3, eq4, eq5,
                inv00, inv01, inv02, inv03, inv04, inv05,
                inv10, inv11, inv12, inv13, inv14, inv15,
                inv20, inv21, inv22, inv23, inv24, inv25,
                inv30, inv31, inv32, inv33, inv34, inv35,
                inv40, inv41, inv42, inv43, inv44, inv45,
                inv50, inv51, inv52, inv53, inv54, inv55,
                inv60, inv61, inv62, inv63, inv64, inv65,
                inv70, inv71, inv72, inv73, inv74, inv75 };

            equipment = new Item[equip_gos.Length];
            equip_buttons = new Button[equip_gos.Length];

            for (int i = 0; i < equipment.Length; i++)
            {
                equipment[i] = new Item();
                equip_buttons[i] = equip_gos[i].GetComponent<Button>();
            }

            // Holder
            if (holder != null)
            {
                holder_rect = holder.GetComponent<RectTransform>();
                holder_item_rect = holder_item.GetComponent<RectTransform>();
                holder.active = holder_item.active = false;

                // Tooltip
                if (tooltip != null)
                {
                    tooltip_rect = tooltip.GetComponent<RectTransform>();
                    tooltip_background_rect = tooltip_background.GetComponent<RectTransform>();
                    tooltip_background.active = tooltip.active = false;
                }
            }

            tooltip_index = -1;
            holding_item.id = 0;

            UpdateActiveGOs();
        }
    }

    public override void Update()
    {
        if (alita_controller != null)
        {
            // Alita Life Bar
            if (life_bar_rect != null)
            {
                float life_percent = alita_controller.GetLifePercent();
                life_bar_rect.x_dist = (uint)(life_percent * life_bar_width);
            }

            // Focus
            if (focus_rect != null)
            {
                float life_percent = alita_controller.GetFocusUnitLifePercent();

                if (focus.active = focus_background.active = life_percent > 0)
                    focus_rect.x_dist = (uint)(life_percent * focus_width);
            }

            // Skill
            if (q_cdr_rect != null)
            {
                float cdr = 1.0f - alita_controller.GetCDR_Q();
                q_cdr_rect.y_dist = skill_height - (uint)(cdr * skill_height);
                q_cdr_rect.y = skill_height + skill_y - q_cdr_rect.y_dist;
            }
        }

        // Inventory
        if (equip_gos != null)
        {
            if (UI.UIHovered() && inventory.active)
            {
                // DEBUG: Add Holding Item on C
                if (Input.GetKeyDown(KeyCode.KEY_C))
                    holding_item.id = 1;

                // DEBUG: Remove Holding Item on V
                if (Input.GetKeyDown(KeyCode.KEY_V))
                    holding_item.id = 0;

                tooltip_index = -1;

                for (int i = 0; i < equip_buttons.Length; i++)
                {
                    switch (equip_buttons[i].state)
                    {
                        case ButtonState.HOVERED:
                            {
                                tooltip_index = i;
                                break;
                            }
                        case ButtonState.L_CLICK:
                            {
                                SwapItemHold(i);
                                break;
                            }
                        default:
                            break;
                    }
                }
            }

            UpdateActiveGOs();
        }
    }

    private void SwapItemHold(int pos)
    {
        // swap data
        int tmp = holding_item.id;
        holding_item.id = equipment[pos].id;
        equipment[pos].id = tmp;

        // casilla, equipment, fuera
    }

    private void UpdateActiveGOs()
    {
        // Inventory
        if (equip_gos != null)
        {
            for (int i = 0; i < equipment.Length; i++)
                equip_gos[i].active = inventory.active ? equipment[i].id > 0 : false;
        }

        // Holder
        if (holder_rect != null)
        {
            Vector2 mouse_pos = Input.GetMousePosition();

            if (holding_item.id > 0)
            {
                holder.active = holder_item.active = true;

                // Mouse Center
                holder_rect.x = holder_item_rect.x = (uint)mouse_pos.x - (holder_rect.x_dist / 2);
                holder_rect.y = holder_item_rect.y = (uint)mouse_pos.y - (holder_rect.y_dist / 2);

                // TODO: Set holder_item Texture to item.id
            }
            else
            {
                holder.active = holder_item.active = false;
            }


            // Tooltip
            if (tooltip_rect != null
                && tooltip_index > 0
                && equipment[tooltip_index].id > 0)
            {
                tooltip_background.active = tooltip.active = true;

                // Mouse Side
                tooltip_background_rect.x = tooltip_rect.x = (uint)mouse_pos.x + (holder_rect.x_dist / 2);
                tooltip_background_rect.y = tooltip_rect.y = (uint)mouse_pos.y - (holder_rect.y_dist / 2);

                // TODO: Show Tooltip Attribute Values
                // TODO: Set Tooltip Attribute Values
            }
            else
            {
                tooltip_background.active = tooltip.active = false;
                // TODO: Hide Tooltip Attribute Values
            }
        }
    }

    public void SwapInventoryActive()
    {
        if (inventory != null)
            inventory.active = !inventory.active;

        if (skill_tree != null)
            skill_tree.active = false;
    }

    public void SwapSkillTreeActive()
    {
        if (skill_tree != null)
            skill_tree.active = !skill_tree.active;

        if (inventory != null)
            inventory.active = false;
    }

    public bool AddItem(Item item)
    {
        if (equip_gos != null)
        {
            for (int i = 6; i < equipment.Length; i++)
            {
                if (equipment[i].id == 0)
                {
                    equipment[i].id = 1;

                    return true;
                }
            }
        }

        return false;
    }
}