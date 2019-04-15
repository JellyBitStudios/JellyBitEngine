using System.Collections;
using System;
using JellyBitEngine;

public class LineOfSight : JellyScript
{
    #region PUBLIC_VARIABLES
    //public float fov = 45.0f;

    public enum SightSensitivity { strict, loose };
    public SightSensitivity sightSensitivity = SightSensitivity.strict;

    public LayerMask layerMask = new LayerMask();

    public GameObject target = null;

    // Gets
    public bool IsTargetSeen
    {
        get { return isTargetSeen; }
    }
    #endregion

    #region PRIVATE_VARIABLES
    private SphereCollider sphereCollider = null;

    private bool isTargetSeen = false;
    #endregion

    public override void Awake()
    {
        sphereCollider = gameObject.GetComponent<SphereCollider>();
    }

    private void UpdateSight()
    {
        switch (sightSensitivity)
        {
            case SightSensitivity.strict:
                isTargetSeen = IsInFOV() && IsInLineOfSight();
                break;

            case SightSensitivity.loose:
                isTargetSeen = IsInFOV() || IsInLineOfSight();
                break;
        }
    }

    private bool IsInFOV()
    {
        // TODO Sandra
        return true;
    }

    private bool IsInLineOfSight()
    {
        RaycastHit hitInfo = new RaycastHit();
        Ray ray = new Ray();
        ray.position = transform.position;
        ray.direction = target.transform.position - transform.position;

        if (Physics.Raycast(ray, out hitInfo, sphereCollider.radius, layerMask, SceneQueryFlags.Static | SceneQueryFlags.Dynamic))
        {
            if (hitInfo.gameObject == null)
                return false;

            // Target?
            if (hitInfo.gameObject == target)
                return true;
        }

        return false;
    }

    public override void OnTriggerStay(Collider collider)
    {
        if (collider == null)
            return;

        // Target?
        if (collider.gameObject.GetLayerID() == target.GetLayerID())
            UpdateSight();
    }

    public override void OnTriggerExit(Collider collider)
    {
        if (collider == null)
            return;

        // Target?
        if (collider.gameObject.GetLayerID() == target.GetLayerID())
            isTargetSeen = false;
    }
}