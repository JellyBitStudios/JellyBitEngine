using System.Collections;
using JellyBitEngine;

public class Waypoint : JellyScript
{
    #region PUBLIC_VARIABLES
    public LayerMask layerMask = new LayerMask(); /// "Enemy"

    public bool IsBlocked
    {
        get { return blockCount > 0u; }
    }
    public bool isOccupied = false;
    #endregion

    #region PRIVATE_VARIABLES
    private uint blockCount = 0u;
    #endregion

    public override void OnTriggerEnter(Collider collider)
    {
        if (collider == null)
            return;

        if (layerMask.HasActive(collider.gameObject.GetLayerID()))
        {
            Debug.Log("Block after ++: " + blockCount);
            ++blockCount;
        }
    }

    public override void OnTriggerExit(Collider collider)
    {
        if (collider == null)
            return;

        if (layerMask.HasActive(collider.gameObject.GetLayerID()))
        {
            Debug.Log("Block after --: " + blockCount);
            --blockCount;
        }
    }
}