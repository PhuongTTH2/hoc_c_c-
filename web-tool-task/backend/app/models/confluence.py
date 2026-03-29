from sqlalchemy import Column, String, DateTime, JSON, Integer, Text
from sqlalchemy.dialects.postgresql import UUID
from sqlalchemy.sql import func
from app.core.database import Base
import uuid

class ConfluenceDetails(Base):
    __tablename__ = "confluence_details"

    id = Column(UUID(as_uuid=True), primary_key=True, default=uuid.uuid4)
    page_id = Column(String(50), unique=True, nullable=False)
    page_title = Column(String(500), nullable=False)
    page_url = Column(String(500))
    space_key = Column(String(50))
    content = Column(Text)
    labels = Column(JSON, default=list)
    raw_response = Column(JSON, default=dict)
    created_at = Column(DateTime(timezone=True), server_default=func.now())