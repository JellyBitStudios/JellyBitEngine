using System.Collections;
using JellyBitEngine;

public class Waypoint : JellyScript
{
    #region PUBLIC_VARIABLES
    public LayerMask layerMask = new LayerMask();

    public bool IsBlocked
    {
        get { return blockCount > 0; }
    }
    public bool isOccupied = false;
    #endregion

    #region PRIVATE_VARIABLES
    private int blockCount = 0;
    #endregion

    public override void OnTriggerEnter(Collider collider)
    {
        if (collider == null)
            return;

        if (collider.gameObject.GetLayer() == "Terrain")
        {
            ++blockCount;
            Debug.Log("Block after ++: " + blockCount);
        }
    }

    public override void OnTriggerExit(Collider collider)
    {
        if (collider == null)
            return;

        if (collider.gameObject.GetLayer() == "Terrain")
        {
            --blockCount;
            Debug.Log("Block after --: " + blockCount);
        }
    }
}