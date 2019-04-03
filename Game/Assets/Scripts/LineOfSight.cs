using System.Collections;
using JellyBitEngine;

public class LineOfSight : JellyScript
{
    #region PUBLIC_VARIABLES
    public float fov = 45.0f;
    public float sightRange = 0.0f; // TODO

    public enum SightSensitivity { strict, loose };
    public SightSensitivity sightSensitivity = SightSensitivity.strict;

    public LayerMask layerMask = new LayerMask();

    // Gets
    public bool IsTargetSeen
    {
        get { return isTargetSeen; }
    }
    public Vector3 LastPositionSeen
    {
        get { return lastPositionSeen; }
    }

    /////
    public GameObject target = null;
    public Transform eyePoint = null;
    #endregion

    #region PRIVATE_VARIABLES
    private Collider sphereCollider = null;

    private bool isTargetSeen = false;
    private Vector3 lastPositionSeen = new Vector3(0.0f, 0.0f, 0.0f);
    #endregion

    public override void Awake()
    {
        sphereCollider = gameObject.GetComponent<Collider>();
    }

    public override void Start()
    {
        lastPositionSeen = transform.position;
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
        ray.position = eyePoint.position;
        ray.direction = target.transform.position - eyePoint.position;
        ray.length = sightRange;

        if (Physics.Raycast(ray, out hitInfo, sightRange, layerMask, SceneQueryFlags.Static | SceneQueryFlags.Dynamic))
        {
            // Target?
            if (hitInfo.gameObject == target)
                return true;
        }

        return false;
    }

    public override void OnTriggerStay(Collider collider)
    {
        // Target?
        if (collider.gameObject == target)
        {
            UpdateSight();

            if (isTargetSeen)
                lastPositionSeen = target.transform.position;
        }
    }
}