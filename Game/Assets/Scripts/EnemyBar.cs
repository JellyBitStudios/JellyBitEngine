using System;
using System.Collections;
using JellyBitEngine;
using JellyBitEngine.UI;


class EnemyBar : JellyScript
{
    public GameObject life_bar;

    private Unit unit = null;
    private float last_current_hp = 0;
    private uint max_x;

    private RectTransform rect = null;

    public override void Start()
    {
        unit = gameObject.GetComponent<Unit>();
        last_current_hp = unit.current_life;

        rect = life_bar.GetComponent<RectTransform>();
        max_x = rect.x_dist;
    }

    public override void Update()
    {
        if (unit == null || rect == null)
        {
            Start();
        }

        if (last_current_hp != unit.current_life)
        {
            last_current_hp = unit.current_life;

            rect.x_dist = (uint)(max_x * (last_current_hp / unit.max_life));

        }
    }
}

