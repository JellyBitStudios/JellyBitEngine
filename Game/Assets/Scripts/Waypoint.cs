using System.Collections;
using JellyBitEngine;

public class Waypoint : JellyScript
{
    #region PUBLIC_VARIABLES
    public LayerMask layerMask = new LayerMask(); /// "Enemies"

    public bool IsBlocked
    {
        get { return blockCount > 0u; }
    }
    public bool IsOccupied
    {
        get { return enemy != null; }
    }
    #endregion

    #region PRIVATE_VARIABLES
    private uint blockCount = 0u;
    private GameObject enemy = null;
    #endregion

    public override void OnTriggerEnter(Collider collider)
    {
        //if (collider.gameObject.GetLayer() != layerMask.)
            ++blockCount;
        // if IsOccupied...
    }

    public override void OnTriggerExit(Collider collider)
    {
        //if (collider.gameObject.GetLayer() != "Enemies")
            --blockCount;
    }
}